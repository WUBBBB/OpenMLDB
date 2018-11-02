# Java SDK 文档

## sdk maven版本

sdk目前发布到公司的仓库

```
    <dependency>
		<groupId>com._4paradigm</groupId>
		<artifactId>rtidb-client</artifactId>
		<version>1.3.0-SNAPSHOT</version>
    </dependency>
```

## 单机版本sdk使用说明

```
package example;

import com._4paradigm.rtidb.client.TableSyncClient;
import com._4paradigm.rtidb.client.ha.RTIDBClientConfig;
import com._4paradigm.rtidb.client.ha.impl.RTIDBSingleNodeClient;
import com._4paradigm.rtidb.client.impl.TableSyncClientImpl;

import io.brpc.client.EndPoint;

public class SingleNodeSyncPutToTable {

    
    public static void main(String[] args) {
        TableSyncClient tableClient = null;        
        EndPoint endpoint = new EndPoint("172.27.128.32:9527");
        RTIDBClientConfig config = new RTIDBClientConfig();
        RTIDBSingleNodeClient snc = new RTIDBSingleNodeClient(config, endpoint);
        try {
            snc.init();
            tableClient = new TableSyncClientImpl(snc);
            boolean ok = tableClient.put(5, 0, 9527, new Object[] {"card0", "merchant0", 9.1d});
            if (ok) {
                System.out.println("put ok");
            }else {
                System.out.println("put failed");
            }
            //释放资源
            snc.close();
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }
}

```

## 集群模式sdk使用

```
package example;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeoutException;

import com._4paradigm.rtidb.client.TableSyncClient;
import com._4paradigm.rtidb.client.TabletException;
import com._4paradigm.rtidb.client.ha.RTIDBClientConfig;
import com._4paradigm.rtidb.client.ha.TableHandler.ReadStrategy;
import com._4paradigm.rtidb.client.ha.impl.RTIDBClusterClient;
import com._4paradigm.rtidb.client.impl.TableSyncClientImpl;
import com._4paradigm.rtidb.client.metrics.TabletMetrics;

public class ClusterSyncPutToTable {

    private static String zookeeper = "172.27.128.33:7181,172.27.128.32:7181,172.27.128.31:7181";
    private static String rootPath = "/trybox";
    
    public static void main(String[] args) {
        /**
         * 使用rtidb ns client 创建包含schema的表
         * 
         */
        // read strategy
        Map<String, ReadStrategy> strategy = new HashMap<String, ReadStrategy>();
        // client 会尽量读取离自己更近的副本
        strategy.put("trans_log", ReadStrategy.kReadLocal);
        RTIDBClientConfig config = new RTIDBClientConfig();
        config.setZkEndpoints(zookeeper);
        config.setZkRootPath(rootPath);
        config.setReadStrategies(strategy);
        // 设置重试次数，2为失败后重试1次，默认值是1
        config.setMaxRetryCnt(2);
        
        //初始化 cluster client
        RTIDBClusterClient cluster = new RTIDBClusterClient(config);
        try {
            cluster.init();
            //创建 同步调用接口
            TableSyncClient tableSyncClient = new TableSyncClientImpl(cluster);
            
            Map<String, Object> row = new HashMap<String, Object>();
            row.put("card", "card0");
            row.put("mcc", "mcc0");
            row.put("amt", 1.1d);
            // 往table trans_log写一条数据
            boolean ok = tableSyncClient.put("trans_log", 1000, row);
            if (ok) {
                System.out.println("write ok");
            }
            cluster.close();
        } catch (TabletException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (TimeoutException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

}

```