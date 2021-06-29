hdfs dfs -mkdir -p /weather/input
hdfs dfs -put /docker-mount/London2013.txt /weather/input
hdfs dfs -rm -r /weather/output

hadoop jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-2.7.0.jar \
-file /docker-mount/mapper.py \
-mapper /docker-mount/mapper.py \
-file /docker-mount/reducer.py \
-reducer /docker-mount/reducer.py \
-input /weather/input/London2013.txt \
-output /weather/output

hdfs dfs -cat /weather/output/* > /docker-mount/result.txt