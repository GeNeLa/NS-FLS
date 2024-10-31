import random
import os
import math

class FLSNode:
    def __init__(self, x, y, z, vx=0, vy=0, vz=0):
        self.x = x
        self.y = y
        self.z = z
        self.vx = vx  # x方向速度
        self.vy = vy  # y方向速度
        self.vz = vz  # z方向速度
        
    def update(self, dt, max_speed=2.0, max_acc=1.0):
        # 添加随机加速度
        ax = random.uniform(-max_acc, max_acc)
        ay = random.uniform(-max_acc, max_acc)
        az = random.uniform(-max_acc/2, max_acc/2)  # z方向加速度较小
        
        # 更新速度
        self.vx += ax * dt
        self.vy += ay * dt
        self.vz += az * dt
        
        # 限制速度
        speed = math.sqrt(self.vx**2 + self.vy**2 + self.vz**2)
        if speed > max_speed:
            self.vx *= max_speed/speed
            self.vy *= max_speed/speed
            self.vz *= max_speed/speed
            
        # 更新位置
        self.x += self.vx * dt
        self.y += self.vy * dt
        self.z += self.vz * dt
        
        # 边界处理
        self.x = max(0, min(100, self.x))
        self.y = max(0, min(100, self.y))
        self.z = max(0, min(20, self.z))
        
        # 碰到边界时反弹
        if self.x <= 0 or self.x >= 100: self.vx *= -0.5
        if self.y <= 0 or self.y >= 100: self.vy *= -0.5
        if self.z <= 0 or self.z >= 20: self.vz *= -0.5

def generate_node_trace(node_id, duration, dt=0.1):
    """
    生成节点轨迹
    params:
        node_id: 节点ID
        duration: 总时长(秒)
        dt: 时间步长(秒)，即采样间隔
    """
    filename = f"trace_node_{node_id}.txt"
    
    # 随机初始化FLS节点
    node = FLSNode(
        x=random.uniform(0, 10),
        y=random.uniform(0, 10),
        z=random.uniform(0, 5),
        vx=random.uniform(-1, 1),
        vy=random.uniform(-1, 1),
        vz=random.uniform(-0.5, 0.5)
    )
    
    # 生成轨迹文件
    with open(filename, 'w') as f:
        t = 0
        while t <= duration:
            # 写入当前状态
            f.write(f"{t:.3f} {node.x:.3f} {node.y:.3f} {node.z:.3f}\n")
            
            # 更新状态
            node.update(dt)
            t += dt

def main():
    # 配置参数
    num_nodes = 50
    duration = 60  # 60秒
    dt = 1      # 采样间隔0.1秒
    
    # 创建并切换到traces目录
    os_directory = "traces"
    if not os.path.exists(os_directory):
        os.makedirs(os_directory)
    os.chdir(os_directory)
    
    # 生成所有节点的轨迹
    for i in range(num_nodes):
        generate_node_trace(i, duration, dt)
        print(f"Generated trace for node {i}")

if __name__ == "__main__":
    main()