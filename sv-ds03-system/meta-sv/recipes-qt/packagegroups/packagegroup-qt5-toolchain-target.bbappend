
USE_RUBY_remove = "qtquick1-dev qtquick1-mkspecs qtquick1-plugins qtquick1-qmlplugins"
USE_RUBY_remove = "qtwebkit-dev qtwebkit-mkspecs qtwebkit-qmlplugins"

RDEPENDS_${PN}_remove = "packagegroup-core-standalone-sdk-target libsqlite3-dev"
RDEPENDS_${PN}_remove = "qt3d-dev qt3d-mkspecs qt3d-qmlplugins"
RDEPENDS_${PN}_remove = "qtconnectivity-dev qtconnectivity-mkspecs qtconnectivity-qmlplugins"
RDEPENDS_${PN}_remove = "qtenginio-dev qtenginio-mkspecs qtenginio-qmlplugins"
RDEPENDS_${PN}_remove = "qtgraphicaleffects-qmlplugins qtimageformats-dev qtimageformats-plugins"
RDEPENDS_${PN}_remove = "qtlocation-dev qtlocation-mkspecs qtlocation-plugins qtlocation-qmlplugins"
RDEPENDS_${PN}_remove = "qtmultimedia-dev qtmultimedia-mkspecs qtmultimedia-plugins qtmultimedia-qmlplugins"
RDEPENDS_${PN}_remove = "qtscript-dev qtscript-mkspecs"
RDEPENDS_${PN}_remove = "qtsensors-dev qtsensors-mkspecs qtsensors-plugins qtsensors-qmlplugins"
RDEPENDS_${PN}_remove = "qtserialport-dev qtserialport-mkspecs"
RDEPENDS_${PN}_remove = "qtsvg-dev qtsvg-mkspecs qtsvg-plugins"
RDEPENDS_${PN}_remove = "qtsystems-dev qtsystems-mkspecs qtsystems-qmlplugins"
RDEPENDS_${PN}_remove = "qtwebsockets-qmlplugins"
RDEPENDS_${PN}_remove = "qtwebchannel-dev qtwebchannel-mkspecs qtwebchannel-qmlplugins"

RDEPENDS_${PN}_remove = "qttranslations-qtquick1 qttranslations-qt qttranslations-qtbase qttranslations-qtconfig qttranslations-qthelp qttranslations-qtconnectivity qttranslations-qmlviewer qttranslations-qtdeclarative qttranslations-qtlocation qttranslations-qtmultimedia qttranslations-qtscript qttranslations-qtxmlpatterns"

RDEPENDS_${PN}_append = "qtwebengine-dev qtwebengine-mkspecs"

RRECOMMENDS_${PN}_remove = "qtquickcontrols-qmlplugins"