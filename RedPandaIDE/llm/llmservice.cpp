/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "llmservice.h"
#include "../settings.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslError>

QString LLMRequestContext::constructPrompt() const
{
    QString prompt;
    
    // Add source code section
    // prompt += "[Current Code Section]\n";
    prompt += "[当前代码片段]\n";
    prompt += sourceCode;
    prompt += "\n\n";
    
    // Add compilation errors section
    // prompt += "[Compilation Errors]\n";
    prompt += "[编译错误]\n";
    
    int count = 0;
    const int MAX_ISSUES = 20; // Limit to first 20 issues
    
    for (const PCompileIssue &issue : issues) {
        if (count >= MAX_ISSUES)
            break;
            
        // Only include errors and warnings
        if (issue->type != CompileIssueType::Error && 
            issue->type != CompileIssueType::Warning)
            continue;
            
        QString typeStr = (issue->type == CompileIssueType::Error) ? "Error" : "Warning";
        
        if (issue->line >= 0 && issue->column >= 0) {
            prompt += QString("%1 at line %2, column %3:\n")
                     .arg(typeStr)
                     .arg(issue->line + 1)
                     .arg(issue->column + 1);
        } else {
            prompt += QString("%1:\n").arg(typeStr);
        }
        
        prompt += issue->description + "\n\n";
        count++;
    }
    
    if (issues.size() > MAX_ISSUES) {
        prompt += QString("... and %1 more issues\n\n").arg(issues.size() - MAX_ISSUES);
    }
    
    // Add question
    // prompt += "[Question]\n";
    prompt += "[问题]\n";
    // prompt += "What is causing these compilation errors? How should the code be modified to fix them?\n";
    prompt += "这些问题是什么原因造成的？应该如何修改代码来修复它们？\n";
    
    return prompt;
}

LLMService::LLMService(QObject *parent)
    : QObject(parent),
      mNetworkManager(new QNetworkAccessManager(this)),
      mCurrentReply(nullptr),
      mRequestActive(false)
{
}

LLMService::~LLMService()
{
    cancel();
}

bool LLMService::isConfigured() const
{
    return pSettings->llm().enabled() &&
           !pSettings->llm().endpoint().isEmpty() &&
           !pSettings->llm().apiKey().isEmpty();
}

QString LLMService::validateConfiguration() const
{
    if (!pSettings->llm().enabled()) {
        return tr("LLM Assistant is not enabled. Please enable it in Tools > Options > LLM Assistant.");
    }
    
    if (pSettings->llm().endpoint().isEmpty()) {
        return tr("API endpoint is not configured. Please set it in Tools > Options > LLM Assistant.");
    }
    
    if (pSettings->llm().apiKey().isEmpty()) {
        return tr("API key is not configured. Please set it in Tools > Options > LLM Assistant.");
    }
    
    QString endpoint = pSettings->llm().endpoint();
    if (!endpoint.startsWith("http://") && !endpoint.startsWith("https://")) {
        return tr("API endpoint must start with http:// or https://");
    }
    
    return QString(); // Empty string means valid
}

void LLMService::diagnoseErrors(const QString &sourceCode,
                                const QString &filename,
                                const QVector<PCompileIssue> &issues)
{
    if (mRequestActive) {
        emit errorOccurred(tr("A request is already in progress."));
        return;
    }
    
    QString validationError = validateConfiguration();
    if (!validationError.isEmpty()) {
        emit errorOccurred(validationError);
        return;
    }
    
    LLMRequestContext context;
    context.sourceCode = sourceCode;
    context.filename = filename;
    context.issues = issues;
    
    QString prompt = context.constructPrompt();
    sendRequest(prompt);
}

void LLMService::sendRequest(const QString &prompt)
{
    QUrl url(pSettings->llm().endpoint());
    QNetworkRequest request(url);
    
    // Set headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", 
                        QString("Bearer %1").arg(pSettings->llm().apiKey()).toUtf8());
    
    // Construct JSON body
    QJsonObject json;
    json["model"] = pSettings->llm().modelName();
    json["stream"] = true;
    
    QJsonArray messages;
    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;
    messages.append(message);
    json["messages"] = messages;
    
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    
    // Send request
    mCurrentReply = mNetworkManager->post(request, data);
    mRequestActive = true;
    mBuffer.clear();
    
    // Set timeout
    mCurrentReply->setProperty("requestTime", QDateTime::currentMSecsSinceEpoch());
    
    // Connect signals
    connect(mCurrentReply, &QNetworkReply::readyRead, this, &LLMService::onReadyRead);
    connect(mCurrentReply, &QNetworkReply::finished, this, &LLMService::onFinished);
    connect(mCurrentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &LLMService::onError);
    connect(mCurrentReply, &QNetworkReply::sslErrors, this, &LLMService::onSslErrors);
}

void LLMService::cancel()
{
    if (mCurrentReply) {
        mCurrentReply->abort();
        mCurrentReply->deleteLater();
        mCurrentReply = nullptr;
    }
    mRequestActive = false;
    mBuffer.clear();
}

void LLMService::onReadyRead()
{
    if (!mCurrentReply)
        return;
        
    QByteArray data = mCurrentReply->readAll();
    mBuffer.append(data);
    
    // Process complete lines
    while (mBuffer.contains('\n')) {
        int newlinePos = mBuffer.indexOf('\n');
        QByteArray line = mBuffer.left(newlinePos);
        mBuffer.remove(0, newlinePos + 1);
        
        if (line.trimmed().isEmpty())
            continue;
            
        parseStreamingChunk(line);
    }
}

void LLMService::parseStreamingChunk(const QByteArray &data)
{
    // SSE format: "data: {json}\n\n"
    QString line = QString::fromUtf8(data).trimmed();
    
    if (!line.startsWith("data: "))
        return;
        
    QString jsonStr = line.mid(6).trimmed(); // Remove "data: " prefix
    
    if (jsonStr == "[DONE]") {
        return; // End of stream marker
    }
    
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isNull() || !doc.isObject())
        return;
        
    QJsonObject obj = doc.object();
    
    // Extract content from choices array
    if (obj.contains("choices") && obj["choices"].isArray()) {
        QJsonArray choices = obj["choices"].toArray();
        if (choices.size() > 0) {
            QJsonObject choice = choices[0].toObject();
            if (choice.contains("delta") && choice["delta"].isObject()) {
                QJsonObject delta = choice["delta"].toObject();
                if (delta.contains("content")) {
                    QString content = delta["content"].toString();
                    if (!content.isEmpty()) {
                        emit responseChunk(content);
                    }
                }
            }
        }
    }
}

void LLMService::onFinished()
{
    mRequestActive = false;
    
    if (!mCurrentReply)
        return;
        
    // Process any remaining data
    if (!mBuffer.isEmpty()) {
        parseStreamingChunk(mBuffer);
        mBuffer.clear();
    }
    
    if (mCurrentReply->error() == QNetworkReply::NoError) {
        emit responseComplete();
    }
    
    mCurrentReply->deleteLater();
    mCurrentReply = nullptr;
}

void LLMService::onError(QNetworkReply::NetworkError error)
{
    if (!mCurrentReply)
        return;
        
    QString errorMsg;
    
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMsg = tr("Connection refused. Please check the API endpoint.");
        break;
    case QNetworkReply::RemoteHostClosedError:
        errorMsg = tr("Remote host closed the connection.");
        break;
    case QNetworkReply::HostNotFoundError:
        errorMsg = tr("Host not found. Please check the API endpoint.");
        break;
    case QNetworkReply::TimeoutError:
        errorMsg = tr("Request timed out. Please try again.");
        break;
    case QNetworkReply::OperationCanceledError:
        errorMsg = tr("Request was cancelled.");
        break;
    case QNetworkReply::SslHandshakeFailedError:
        errorMsg = tr("SSL handshake failed.");
        break;
    case QNetworkReply::TemporaryNetworkFailureError:
        errorMsg = tr("Temporary network failure. Please try again.");
        break;
    case QNetworkReply::NetworkSessionFailedError:
        errorMsg = tr("Network session failed.");
        break;
    case QNetworkReply::BackgroundRequestNotAllowedError:
        errorMsg = tr("Background request not allowed.");
        break;
    case QNetworkReply::TooManyRedirectsError:
        errorMsg = tr("Too many redirects.");
        break;
    case QNetworkReply::InsecureRedirectError:
        errorMsg = tr("Insecure redirect detected.");
        break;
    case QNetworkReply::ContentAccessDenied:
        errorMsg = tr("Access denied. Please check your API key.");
        break;
    case QNetworkReply::ContentNotFoundError:
        errorMsg = tr("Content not found (404).");
        break;
    case QNetworkReply::AuthenticationRequiredError:
        errorMsg = tr("Authentication required. Please check your API key.");
        break;
    case QNetworkReply::InternalServerError:
        errorMsg = tr("Internal server error (500).");
        break;
    case QNetworkReply::ServiceUnavailableError:
        errorMsg = tr("Service unavailable (503). Please try again later.");
        break;
    default:
        errorMsg = tr("Network error: %1").arg(mCurrentReply->errorString());
        break;
    }
    
    emit errorOccurred(errorMsg);
}

void LLMService::onSslErrors(const QList<QSslError> &errors)
{
    // For now, ignore SSL errors (not recommended for production)
    // In production, you should properly validate certificates
    if (mCurrentReply) {
        mCurrentReply->ignoreSslErrors();
    }
}
