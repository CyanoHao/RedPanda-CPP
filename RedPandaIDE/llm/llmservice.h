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
#ifndef LLMSERVICE_H
#define LLMSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QVector>
#include "../common.h"

class LLMResultWindow;

struct LLMRequestContext {
    QString sourceCode;
    QString filename;
    QVector<PCompileIssue> issues;
    QString constructPrompt() const;
};

class LLMService : public QObject
{
    Q_OBJECT
public:
    explicit LLMService(QObject *parent = nullptr);
    ~LLMService();

    bool isConfigured() const;
    QString validateConfiguration() const;
    
    void diagnoseErrors(const QString &sourceCode, 
                       const QString &filename,
                       const QVector<PCompileIssue> &issues);
    
    void cancel();

signals:
    void responseChunk(const QString &chunk);
    void responseComplete();
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onFinished();
    void onError(QNetworkReply::NetworkError error);
    void onSslErrors(const QList<QSslError> &errors);

private:
    QString constructPrompt(const LLMRequestContext &context) const;
    QString formatIssues(const QVector<PCompileIssue> &issues) const;
    void sendRequest(const QString &prompt);
    void parseStreamingChunk(const QByteArray &data);
    
    QNetworkAccessManager *mNetworkManager;
    QNetworkReply *mCurrentReply;
    QByteArray mBuffer;
    bool mRequestActive;
};

#endif // LLMSERVICE_H
