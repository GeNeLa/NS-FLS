def generate_static_point_traces():
    """
    为每个静态点生成轨迹文件
    每行的三个值分别是x,y,z坐标
    行号对应节点编号（从1开始）
    """
    # 从coord.txt读取三维坐标数据
    coordinates = []
    with open('coord.txt', 'r') as f:
        coordinates = [tuple(map(int, line.split())) for line in f if line.strip()]
    
    # 为每个点生成轨迹文件
    for node_id, (x, y, z) in enumerate(coordinates, 1):  # 从1开始编号
        filename = f'trace_node_{node_id}'
        with open(filename, 'w') as f:
            # 对于每一秒（0到65），输出点的三维坐标
            for t in range(66):  # 0到65，共66个时间点
                f.write(f"{t} {x} {y} {z}\n")
        
        print(f"已生成节点 {node_id} 的轨迹文件")

if __name__ == "__main__":
    try:
        generate_static_point_traces()
        print("所有轨迹文件生成完成")
    except FileNotFoundError:
        print("找不到 coord.txt 文件")
    except Exception as e:
        print(f"处理过程中出现错误: {str(e)}")