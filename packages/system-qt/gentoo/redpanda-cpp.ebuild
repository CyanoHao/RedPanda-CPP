EAPI=8
DESCRIPTION="A fast, lightweight, open source, and cross platform C++ IDE"
HOMEPAGE="https://github.com/royqh1979/RedPanda-CPP"
SRC_URI="RedPanda-CPP.tar.gz"
S="${WORKDIR}/RedPanda-CPP"
LICENSE="GPL-3"
SLOT="0"
RDEPEND="dev-qt/qtbase[opengl]
         dev-qt/qtsvg"
DEPEND="${RDEPEND}"
BDEPEND="dev-qt/qttools"

src_configure() {
  qmake6 PREFIX=/usr .
}

src_install() {
  make INSTALL_ROOT="${D}" install
}
