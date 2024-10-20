import random
import os

def generate_node_trace(node_id, duration):
    filename = f"trace_node_{node_id}.txt"
    with open(filename, 'w') as f:
        x, y, z = random.uniform(0, 100), random.uniform(0, 100), random.uniform(0, 20)
        for t in range(duration + 1):
            # 随机生成新的位置，但保持一定的连续性
            x += random.uniform(-5, 5)
            y += random.uniform(-5, 5)
            z += random.uniform(-2, 2)
            
            # 确保坐标在合理范围内
            x = max(0, min(100, x))
            y = max(0, min(100, y))
            z = max(0, min(20, z))
            
            f.write(f"{t:.1f} {x:.2f} {y:.2f} {z:.2f}\n")

def main():
    num_nodes = 50
    duration = 60  # 60秒

    # 创建traces目录(如果不存在)
    os_directory = "traces"
    if not os.path.exists(os_directory):
        os.makedirs(os_directory)

    # 切换到traces目录
    os.chdir(os_directory)

    for i in range(num_nodes):
        generate_node_trace(i, duration)
        print(f"Generated trace for node {i}")

if __name__ == "__main__":
    main()
