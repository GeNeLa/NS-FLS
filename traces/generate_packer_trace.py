import random
import os

def generate_packet_trace(node_id, simulation_time, interval, num_nodes):
    filename = f"packet_trace_node_{node_id}.txt"
    with open(filename, 'w') as f:
        current_time = 0
        while current_time < simulation_time:
            size = random.randint(100, 1024)  # 随机包大小,100到1024字节
            destination = f"10.1.1.{random.randint(1, num_nodes)}"  # 随机目标IP
            f.write(f"{current_time:.3f} {size} {destination}\n")
            current_time += interval

def main():
    num_nodes = 10  # 节点数量
    simulation_time = 20.0  # 仿真时间(秒)
    interval = 1  # 30毫秒的间隔

    # 创建traces目录(如果不存在)
    os_directory = "interTest"
    if not os.path.exists(os_directory):
        os.makedirs(os_directory)

    # 切换到traces目录
    os.chdir(os_directory)

    for i in range(0, num_nodes):  # 从1开始，到50
        generate_packet_trace(i, simulation_time, interval, num_nodes)
        print(f"Generated packet trace for node {i}")

if __name__ == "__main__":
    main()
