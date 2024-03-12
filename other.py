import re
import matplotlib.pyplot as plt

receive_data_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:OnData\(\): \[INFO \] Consumer < DATA for \/ndn.blockchain\/(\d+)\/update_SpecificBcS"
finish_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:handleData_ReceivedBlock\(\): \[DEBUG\] 完成目标: 100"
finish_pull_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:OnInterest\(\): \[INFO \] after finish_pull:(\d+)"
new_block_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:SendPacket\(\): \[INFO \] Consumer > Interest for \/ndnblockchain\/request\/newblock\/(\d+)\/"
receive_new_block_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:handleData_ReceivedBlock\(\): \[DEBUG\] 完成目标: 101"
hop_count_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:OnInterest\(\): \[DEBUG\] ====Hop count: (\d+)"
data_file = "./tp40-mul.log"
node_num = 20
finish_block_nodes_time = {}
receive_new_block_nodes_time = {}
finish_nodes_time = {}
receive_block_detail = {}
layer_out_dict = {}
test_sentence = (
    "+5.805290656s 3 ndnBlockchainApp:handleData_ReceivedBlock(): [DEBUG] 完成目标: 100"
)
test_match = re.match(finish_pattern, test_sentence)
print(test_match)
start_time = 5
new_block_start_time = 10.0
with open(data_file, "r", encoding="UTF-8") as file:
    for line in file:
        finish_pull_match = re.match(finish_pull_pattern, line)
        finish_match = re.match(finish_pattern, line)
        receive_block_match = re.match(receive_data_pattern, line)
        hop_count_match = re.match(hop_count_pattern, line)
        new_block_match = re.match(new_block_pattern, line)
        receive_new_block_match = re.match(receive_new_block_pattern, line)
        if receive_block_match:
            node_id = int(receive_block_match.group(2))
            receive_time = float(receive_block_match.group(1))
            receive_block = int(receive_block_match.group(3))
            if node_id not in receive_block_detail.keys():
                receive_block_detail[node_id] = {}
            receive_block_detail[node_id] = {receive_block: receive_time}
        elif hop_count_match:
            node_id = int(hop_count_match.group(2))
            hop_count = int(hop_count_match.group(3))
            if hop_count not in layer_out_dict.keys():
                layer_out_dict[hop_count] = {}
            layer_out_dict[hop_count][node_id] = 0.0
        elif finish_pull_match:
            node_id = int(finish_pull_match.group(2))
            finish_pull_time = float(finish_pull_match.group(1))
            finish_pull_height = int(finish_pull_match.group(3))
            finish_nodes_time[node_id] = finish_pull_time
            for layer in layer_out_dict.keys():
                if node_id in layer_out_dict[layer].keys():
                    layer_out_dict[layer][node_id] = finish_pull_time - start_time
        elif finish_match:
            node_id = int(finish_match.group(2))
            finish_time = float(finish_match.group(1))
            finish_block_nodes_time[node_id] = finish_time
        elif new_block_match:
            pass
        elif receive_new_block_match:
            node_id = int(receive_new_block_match.group(2))
            receive_time = float(receive_new_block_match.group(1))
            receive_new_block_nodes_time[node_id] = receive_time
            pass


print("++++++++++++++++++")
for key, value in finish_block_nodes_time.items():
    print(f"Node{key} finished getting block at {value}s")
block_sorted_out = sorted(finish_block_nodes_time.items(), key=lambda x: x[1])
new_block_sorted_out = sorted(receive_new_block_nodes_time.items(), key=lambda x: x[1])
print(
    "finished getting blocks at "
    + str(block_sorted_out[-1][1])
    + ", takes "
    + str(block_sorted_out[-1][1] - start_time)
    + "s"
)
print(
    "finished receiving new blocks at "
    + str(new_block_sorted_out[-1][1])
    + ", takes "
    + str(new_block_sorted_out[-1][1] - new_block_start_time)
    + "s"
)
