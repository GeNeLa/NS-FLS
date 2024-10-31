import random
import os

def generate_test_traces():
    # 创建traces目录
    if not os.path.exists("interTest"):
        os.makedirs("interTest")
    
    num_nodes = 10  # 10个节点
    duration = 10   # 10秒
    area_size = 100 # 活动范围100x100x20

    # 为每个节点生成轨迹
    for node_id in range(num_nodes):
        filename = f"traces/trace_node_{node_id}.txt"
        
        # 生成初始位置
        current_pos = {
            'x': random.uniform(0, area_size),
            'y': random.uniform(0, area_size),
            'z': random.uniform(0, 20)
        }

        with open(filename, 'w') as f:
            # 每秒记录一个位置
            for t in range(duration + 1):  # +1确保包含第10秒
                # 写入当前位置
                f.write(f"{t:.1f} {current_pos['x']:.3f} {current_pos['y']:.3f} {current_pos['z']:.3f}\n")
                
                # 计算下一个位置（添加随机移动）
                current_pos = {
                    'x': max(0, min(area_size, current_pos['x'] + random.uniform(-10, 10))),
                    'y': max(0, min(area_size, current_pos['y'] + random.uniform(-10, 10))),
                    'z': max(0, min(20, current_pos['z'] + random.uniform(-2, 2)))
                }

        print(f"Generated trace for node {node_id}")

if __name__ == "__main__":
    generate_test_traces()
    print("\nTrace files have been generated in the 'traces' directory.")