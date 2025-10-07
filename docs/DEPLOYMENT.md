# EpiphanyDB 部署指南

本文档提供了 EpiphanyDB 在不同环境中的部署方法和最佳实践。

## 目录

- [Docker 部署](#docker-部署)
- [Kubernetes 部署](#kubernetes-部署)
- [裸机部署](#裸机部署)
- [云平台部署](#云平台部署)
- [监控和日志](#监控和日志)
- [安全配置](#安全配置)
- [性能调优](#性能调优)
- [故障排除](#故障排除)

## Docker 部署

### 快速开始

使用 Docker Compose 快速启动 EpiphanyDB：

```bash
# 克隆项目
git clone https://github.com/your-org/epiphanyDB.git
cd epiphanyDB

# 启动服务
docker-compose up -d

# 查看服务状态
docker-compose ps

# 查看日志
docker-compose logs -f epiphanydb
```

### 单容器部署

```bash
# 构建镜像
docker build -t epiphanydb:latest .

# 运行容器
docker run -d \
  --name epiphanydb \
  -p 5432:5432 \
  -p 8080:8080 \
  -v epiphany_data:/var/lib/epiphanydb \
  -v epiphany_logs:/var/log/epiphanydb \
  -e EPIPHANY_LOG_LEVEL=info \
  -e EPIPHANY_MAX_CONNECTIONS=100 \
  epiphanydb:latest
```

### 环境变量配置

| 变量名 | 默认值 | 描述 |
|--------|--------|------|
| `EPIPHANY_MODE` | `server` | 运行模式 |
| `EPIPHANY_LOG_LEVEL` | `info` | 日志级别 |
| `EPIPHANY_MAX_CONNECTIONS` | `100` | 最大连接数 |
| `EPIPHANY_MEMORY_LIMIT` | `1G` | 内存限制 |
| `EPIPHANY_ENABLE_METRICS` | `false` | 启用指标收集 |

### 数据持久化

```bash
# 创建数据卷
docker volume create epiphany_data
docker volume create epiphany_logs

# 备份数据
docker run --rm -v epiphany_data:/data -v $(pwd):/backup \
  alpine tar czf /backup/epiphany_backup.tar.gz -C /data .

# 恢复数据
docker run --rm -v epiphany_data:/data -v $(pwd):/backup \
  alpine tar xzf /backup/epiphany_backup.tar.gz -C /data
```

## Kubernetes 部署

### 命名空间和资源

```yaml
# namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: epiphanydb
  labels:
    name: epiphanydb
```

### ConfigMap 配置

```yaml
# configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: epiphanydb-config
  namespace: epiphanydb
data:
  epiphany.conf: |
    [server]
    mode = server
    data_directory = /var/lib/epiphanydb/data
    log_directory = /var/log/epiphanydb
    
    [network]
    listen_address = 0.0.0.0
    port = 5432
    max_connections = 200
    
    [performance]
    memory_limit = 2G
    worker_threads = auto
```

### Deployment 配置

```yaml
# deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: epiphanydb
  namespace: epiphanydb
spec:
  replicas: 3
  selector:
    matchLabels:
      app: epiphanydb
  template:
    metadata:
      labels:
        app: epiphanydb
    spec:
      containers:
      - name: epiphanydb
        image: epiphanydb/epiphanydb:latest
        ports:
        - containerPort: 5432
        - containerPort: 8080
        env:
        - name: EPIPHANY_MODE
          value: "server"
        - name: EPIPHANY_LOG_LEVEL
          value: "info"
        volumeMounts:
        - name: data
          mountPath: /var/lib/epiphanydb
        - name: logs
          mountPath: /var/log/epiphanydb
        - name: config
          mountPath: /opt/epiphanydb/etc
        resources:
          requests:
            memory: "1Gi"
            cpu: "500m"
          limits:
            memory: "2Gi"
            cpu: "1000m"
        livenessProbe:
          exec:
            command:
            - /opt/epiphanydb/bin/healthcheck.sh
            - --quick
          initialDelaySeconds: 60
          periodSeconds: 30
        readinessProbe:
          exec:
            command:
            - /opt/epiphanydb/bin/healthcheck.sh
            - --quick
          initialDelaySeconds: 30
          periodSeconds: 10
      volumes:
      - name: data
        persistentVolumeClaim:
          claimName: epiphanydb-data
      - name: logs
        persistentVolumeClaim:
          claimName: epiphanydb-logs
      - name: config
        configMap:
          name: epiphanydb-config
```

### Service 配置

```yaml
# service.yaml
apiVersion: v1
kind: Service
metadata:
  name: epiphanydb-service
  namespace: epiphanydb
spec:
  selector:
    app: epiphanydb
  ports:
  - name: database
    port: 5432
    targetPort: 5432
  - name: metrics
    port: 8080
    targetPort: 8080
  type: ClusterIP

---
apiVersion: v1
kind: Service
metadata:
  name: epiphanydb-external
  namespace: epiphanydb
spec:
  selector:
    app: epiphanydb
  ports:
  - name: database
    port: 5432
    targetPort: 5432
    nodePort: 30432
  type: NodePort
```

### 持久化存储

```yaml
# pvc.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: epiphanydb-data
  namespace: epiphanydb
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 100Gi
  storageClassName: fast-ssd

---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: epiphanydb-logs
  namespace: epiphanydb
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 20Gi
  storageClassName: standard
```

## 裸机部署

### 系统要求

- **操作系统**: Ubuntu 20.04+ / CentOS 8+ / RHEL 8+
- **CPU**: 4+ 核心
- **内存**: 8GB+ RAM
- **存储**: 100GB+ SSD
- **网络**: 1Gbps+

### 安装步骤

```bash
# 1. 安装依赖
sudo apt update
sudo apt install -y build-essential cmake git pkg-config

# 2. 创建用户
sudo useradd -r -s /bin/bash -d /opt/epiphanydb epiphany
sudo mkdir -p /opt/epiphanydb /var/lib/epiphanydb /var/log/epiphanydb
sudo chown -R epiphany:epiphany /opt/epiphanydb /var/lib/epiphanydb /var/log/epiphanydb

# 3. 编译安装
git clone https://github.com/your-org/epiphanyDB.git
cd epiphanyDB
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/epiphanydb ..
make -j$(nproc)
sudo make install

# 4. 配置服务
sudo cp scripts/epiphanydb.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable epiphanydb
sudo systemctl start epiphanydb
```

### 系统服务配置

```ini
# /etc/systemd/system/epiphanydb.service
[Unit]
Description=EpiphanyDB Database Server
After=network.target
Wants=network.target

[Service]
Type=forking
User=epiphany
Group=epiphany
ExecStart=/opt/epiphanydb/bin/epiphanydb-server -D /var/lib/epiphanydb
ExecReload=/bin/kill -HUP $MAINPID
KillMode=mixed
KillSignal=SIGINT
TimeoutSec=0

# Resource limits
LimitNOFILE=65536
LimitNPROC=32768

# Security
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ReadWritePaths=/var/lib/epiphanydb /var/log/epiphanydb

[Install]
WantedBy=multi-user.target
```

## 云平台部署

### AWS 部署

#### EC2 实例

```bash
# 启动 EC2 实例
aws ec2 run-instances \
  --image-id ami-0c55b159cbfafe1d0 \
  --instance-type m5.xlarge \
  --key-name my-key \
  --security-group-ids sg-12345678 \
  --subnet-id subnet-12345678 \
  --user-data file://user-data.sh
```

#### RDS 兼容部署

```yaml
# CloudFormation 模板
AWSTemplateFormatVersion: '2010-09-09'
Resources:
  EpiphanyDBInstance:
    Type: AWS::RDS::DBInstance
    Properties:
      DBInstanceClass: db.r5.xlarge
      Engine: postgres
      MasterUsername: epiphany
      MasterUserPassword: !Ref DBPassword
      AllocatedStorage: 100
      StorageType: gp2
      VPCSecurityGroups:
        - !Ref DBSecurityGroup
```

### Google Cloud 部署

```bash
# 创建 GKE 集群
gcloud container clusters create epiphanydb-cluster \
  --num-nodes=3 \
  --machine-type=n1-standard-4 \
  --disk-size=100GB \
  --enable-autoscaling \
  --min-nodes=1 \
  --max-nodes=10

# 部署应用
kubectl apply -f k8s/
```

### Azure 部署

```bash
# 创建资源组
az group create --name epiphanydb-rg --location eastus

# 创建 AKS 集群
az aks create \
  --resource-group epiphanydb-rg \
  --name epiphanydb-cluster \
  --node-count 3 \
  --node-vm-size Standard_D4s_v3 \
  --enable-addons monitoring
```

## 监控和日志

### Prometheus 监控

```yaml
# prometheus-config.yaml
global:
  scrape_interval: 15s

scrape_configs:
  - job_name: 'epiphanydb'
    static_configs:
      - targets: ['epiphanydb:8080']
```

### Grafana 仪表板

```json
{
  "dashboard": {
    "title": "EpiphanyDB Monitoring",
    "panels": [
      {
        "title": "Query Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(epiphanydb_queries_total[5m])"
          }
        ]
      }
    ]
  }
}
```

### 日志聚合

```yaml
# fluentd-config.yaml
<source>
  @type tail
  path /var/log/epiphanydb/*.log
  pos_file /var/log/fluentd/epiphanydb.log.pos
  tag epiphanydb.*
  format json
</source>

<match epiphanydb.**>
  @type elasticsearch
  host elasticsearch
  port 9200
  index_name epiphanydb
</match>
```

## 安全配置

### SSL/TLS 配置

```bash
# 生成证书
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
  -keyout epiphanydb.key -out epiphanydb.crt

# 配置 SSL
echo "ssl = on" >> /etc/epiphanydb/epiphanydb.conf
echo "ssl_cert_file = '/etc/ssl/certs/epiphanydb.crt'" >> /etc/epiphanydb/epiphanydb.conf
echo "ssl_key_file = '/etc/ssl/private/epiphanydb.key'" >> /etc/epiphanydb/epiphanydb.conf
```

### 防火墙配置

```bash
# Ubuntu/Debian
sudo ufw allow 5432/tcp
sudo ufw allow 8080/tcp
sudo ufw enable

# CentOS/RHEL
sudo firewall-cmd --permanent --add-port=5432/tcp
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --reload
```

### 访问控制

```conf
# pg_hba.conf 样式配置
# TYPE  DATABASE        USER            ADDRESS                 METHOD
local   all             all                                     trust
host    all             all             127.0.0.1/32            md5
host    all             all             ::1/128                 md5
host    all             all             10.0.0.0/8              md5
```

## 性能调优

### 内存配置

```conf
# 内存相关配置
shared_buffers = 256MB          # 25% of RAM
effective_cache_size = 1GB      # 75% of RAM
work_mem = 16MB                 # RAM / max_connections / 4
maintenance_work_mem = 64MB     # RAM / 16
```

### 存储优化

```conf
# 存储相关配置
checkpoint_completion_target = 0.9
wal_buffers = 16MB
default_statistics_target = 100
random_page_cost = 1.1          # For SSD
effective_io_concurrency = 200  # For SSD
```

### 连接池配置

```conf
# 连接相关配置
max_connections = 200
superuser_reserved_connections = 3
tcp_keepalives_idle = 600
tcp_keepalives_interval = 30
tcp_keepalives_count = 3
```

## 故障排除

### 常见问题

#### 连接问题

```bash
# 检查服务状态
systemctl status epiphanydb

# 检查端口监听
netstat -tlnp | grep 5432

# 检查日志
tail -f /var/log/epiphanydb/epiphanydb.log
```

#### 性能问题

```sql
-- 查看慢查询
SELECT query, mean_time, calls 
FROM pg_stat_statements 
ORDER BY mean_time DESC 
LIMIT 10;

-- 查看锁等待
SELECT * FROM pg_locks WHERE NOT granted;
```

#### 存储问题

```bash
# 检查磁盘空间
df -h /var/lib/epiphanydb

# 检查 I/O 状态
iostat -x 1

# 检查数据库大小
du -sh /var/lib/epiphanydb/*
```

### 日志分析

```bash
# 错误日志分析
grep ERROR /var/log/epiphanydb/epiphanydb.log | tail -20

# 慢查询日志
grep "slow query" /var/log/epiphanydb/epiphanydb.log

# 连接日志
grep "connection" /var/log/epiphanydb/epiphanydb.log
```

### 备份和恢复

```bash
# 创建备份
pg_dump -h localhost -p 5432 -U epiphany epiphanydb > backup.sql

# 恢复备份
psql -h localhost -p 5432 -U epiphany epiphanydb < backup.sql

# 物理备份
pg_basebackup -h localhost -p 5432 -U epiphany -D /backup/base -Ft -z -P
```

## 最佳实践

### 部署清单

- [ ] 系统资源充足（CPU、内存、存储）
- [ ] 网络配置正确
- [ ] 安全配置完成（SSL、防火墙、访问控制）
- [ ] 监控和日志配置
- [ ] 备份策略制定
- [ ] 性能基准测试
- [ ] 故障恢复计划
- [ ] 文档更新

### 运维建议

1. **定期备份**: 建立自动化备份策略
2. **监控告警**: 配置关键指标告警
3. **性能调优**: 定期分析性能瓶颈
4. **安全更新**: 及时应用安全补丁
5. **容量规划**: 监控资源使用趋势
6. **文档维护**: 保持部署文档更新

## 获取帮助

- **文档**: [https://epiphanydb.io/docs](https://epiphanydb.io/docs)
- **社区**: [https://github.com/your-org/epiphanyDB/discussions](https://github.com/your-org/epiphanyDB/discussions)
- **问题报告**: [https://github.com/your-org/epiphanyDB/issues](https://github.com/your-org/epiphanyDB/issues)
- **邮件列表**: epiphanydb@example.com