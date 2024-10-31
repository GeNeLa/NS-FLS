from flask import Flask, request, jsonify
from flask_cors import CORS
import subprocess
import json
import os

app = Flask(__name__)
CORS(app)  # 允许跨域请求，这样前端可以访问不同端口的后端

# 定义路由，处理仿真请求
@app.route('/run-simulation', methods=['POST'])
def run_simulation():
    try:
        # 从前端接收JSON格式的配置参数
        config = request.json
        print("Received configuration:", config)
        
        # 构建NS-3命令行参数
        # 这些参数要与你的NS-3程序中的 cmd.AddValue() 对应
        sim_args = [
            './ns3',  # NS-3执行命令
            'run', 
            'scratch/fls-simulation',  # 你的仿真程序路径
            # 使用 get() 方法提供默认值，避免参数缺失导致错误
            f'--nNodes={config.get("nodeCount", 10)}',
            f'--simulationTime={config.get("simulationTime", 10)}',
            f'--txPower={config.get("txPower", 20)}',
            f'--frequency={config.get("frequency", 5)}',
            f'--channelWidth={config.get("channelWidth", 80)}',
            f'--propagationModel={config.get("propagationModel", "LogDistance")}'
        ]
        
        print("Running command:", ' '.join(sim_args))
        
        # 使用subprocess运行NS-3程序
        process = subprocess.Popen(
            sim_args,
            cwd='~/ns-3-dev/scratch/FLS/fls-simulation',  # NS-3安装路径
            stdout=subprocess.PIPE,  # 捕获标准输出
            stderr=subprocess.PIPE   # 捕获错误输出
        )
        
        # 等待程序执行完成并获取输出
        stdout, stderr = process.communicate()
        
        # 检查是否执行成功
        if process.returncode != 0:
            print("Simulation failed:", stderr.decode())
            return jsonify({
                'status': 'error',
                'message': stderr.decode()
            }), 500
            
        # 读取仿真结果文件
        try:
            with open('/path/to/your/ns-3/simulation-results.json', 'r') as f:
                results = json.load(f)
        except FileNotFoundError:
            # 如果结果文件不存在，尝试从输出中解析结果
            results = {
                'flowStats': [],
                'timeSeriesData': []
            }
            
            # 这里可以添加自定义的输出解析逻辑
            lines = stdout.decode().split('\n')
            for line in lines:
                if line.startswith('Flow'):
                    # 解析流统计信息
                    # 例如: Flow 1: Throughput = 5.2 Mbps
                    pass
                elif line.startswith('Time'):
                    # 解析时间序列数据
                    # 例如: Time 1.0: Packets = 100
                    pass
        
        # 返回成功结果给前端
        return jsonify({
            'status': 'success',
            'results': results
        })
        
    except Exception as e:
        print("Error occurred:", str(e))
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500

# 健康检查接口
@app.route('/health', methods=['GET'])
def health_check():
    return jsonify({'status': 'healthy'})

# 启动服务器
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)