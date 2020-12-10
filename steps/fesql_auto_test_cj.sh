#!/usr/bin/env bash

ROOT_DIR=`pwd`
ulimit -c unlimited

echo "ROOT_DIR:${ROOT_DIR}"
sh tools/install_fesql_cj.sh
sh steps/gen_code.sh
mkdir -p ${ROOT_DIR}/build  && cd ${ROOT_DIR}/build && cmake ..
if [ -z "${FEDEV}" ]; then
    MAVEN_OPTS="-DsocksProxyHost=127.0.0.1 -DsocksProxyPort=1080" make -j5 sql_javasdk_package || { echo "compile error"; exit 1; }
else
    make -j16 || { echo "compile error"; exit 1; }
fi
cd ${ROOT_DIR}
test -d ~/tmp/rambuild/ut_zookeeper && rm -rf ~/tmp/rambuild/ut_zookeeper/*
cp steps/cjzoo.cfg thirdsrc/zookeeper-3.4.14/conf/zoo.cfg
cd thirdsrc/zookeeper-3.4.14
#netstat -atn | grep 6181 | awk '{print $NF}' | awk -F '/' '{print $1}'| xargs kill -9
lsof -i:6181 |grep 6181| awk '{print $2}'| xargs kill -9
./bin/zkServer.sh start && cd $ROOT_DIR
sleep 5
cd onebox && sh start_onebox_on_rambuild_cj.sh && cd $ROOT_DIR
sleep 5
case_xml=test_v1.xml
cd ${ROOT_DIR}/src/sdk/java/
MAVEN_OPTS="-DsocksProxyHost=127.0.0.1 -DsocksProxyPort=1080" mvn install -Dmaven.test.skip=true
cd ${ROOT_DIR}/src/sdk/java/fesql-auto-test-java
MAVEN_OPTS="-DsocksProxyHost=127.0.0.1 -DsocksProxyPort=1080" mvn test -DsuiteXmlFile=test_suite/${case_xml}