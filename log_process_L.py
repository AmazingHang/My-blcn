import csv
import re
import pandas as pd

data_file = "./ws50out.log"
output_file = "./ws50out_output.xlsx"
#
# send_interest_pattern = r"\+(\d+\.\d+)s (\d+) ndn\.Consumer:SendPacket\(\): \[INFO \] > Interest for (\d+)"
# receive_data_pattern = r"\+(\d+\.\d+)s (\d+) ndn.Consumer:OnData\(\): \[INFO \] < DATA for (\d+)"
# retx_interest_pattern = r""
# receive_interest_pattern = r"\+(\d+\.\d+)s (\d+) ndn.Producer:OnInterest\(\): \[INFO \] node\(\d+\) responding with Data: /(.*?)/seq=(\d+)"
# hop_count_pattern = r"\+(\d+\.\d+)s (\d+) ndn.Consumer:OnData\(\): \[DEBUG\] Hop count: (\d+)"
# time_out_pattern = r"\+(\d+\.\d+)s (\d+) ndn.(.*?):OnTimeout\(\d+\)"
send_interest_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:SendPacket\(\): \[INFO \] Consumer > Interest for \/ndn\.blockchain\/(.*?)\/(\d+)\/pull_BlockchainStatus"
send_specific_interest_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:SendPacket\(\): \[INFO \] Consumer > Interest for \/ndn\.blockchain/(\d+)\/update_SpecificBcS"
receive_data_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:OnData\(\): \[INFO \] Consumer < DATA for \/ndn\.blockchain\/(\d+)\/update_SpecificBcS"
finish_pattern = (
    r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:handleData_ReceivedBlock\(\): \[DEBUG\] 完成目标"
)
receive_interest_pattern = r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:OnInterest\(\): \[INFO \] Recieved Interest \/ndn\.blockchain\/(\d+)\/update_SpecificBcS"
hop_count_pattern = (
    r"\+(\d+\.\d+)s (\d+) ndnBlockchainApp:OnData\(\): \[DEBUG\] Hop count: (\d+)"
)
time_out_pattern = r"\+(\d+\.\d+)s (\d+) ndn.(.*?):OnTimeout\(\d+\)"
# +0.220560000s 1 ndn.Consumer:OnData(): [DEBUG] Hop count: 3
test_sentence = (
    "+5.401947200s 4 ndnBlockchainApp:handleData_ReceivedBlock(): [DEBUG] 完成目标"
)
send_match = re.match(finish_pattern, test_sentence)
print(send_match)


def flatter(finish_node):
    for node_data, data_content in node_summary_dict.items():
        if node_data == finish_node:
            for id, info in data_content.items():
                row = {
                    "data name": id,
                    "node id": node_data,
                    "Send interest time": info["Send interest time"],
                    "Get Data time": info["Get Data time"],
                    "All time": info["All time"],
                }
                flattered_node_data.append(row)
            a = sorted(
                data_content.items(),
                key=lambda d: float(d[1]["Get Data time"]),
                reverse=False,
            )
            start_time_stamp = a[0][1]["Send interest time"]
            end_time_stamp = a[-1][1]["Get Data time"]
            ninety_five = int(len(data_content) * 0.95)
            ninety_five_time = a[ninety_five - 1][1]["Get Data time"]
            s = {
                "node id": node_data,
                "Start time": start_time_stamp,
                "End time": end_time_stamp,
                "All time": end_time_stamp - start_time_stamp,
                "Start height": a[0][0],
                "End height": a[-1][0],
                "Data num": len(a),
                "95% time": ninety_five_time - start_time_stamp,
            }
            average_data.append(ninety_five_time - start_time_stamp)
            flattered_node_summary.append(s)
    node_summary_dict.pop(finish_node)


send_interest_info = {}
send_interest_node_info = {}
get_interest_info = {}
time_info = []
node_time_info = []
summary_dict = {}
node_summary_dict = {}
flattered_node_summary = []
flattered_data = []
flattered_propagation_data = []
flattered_node_data = []
average_data = []
# consumer_node_num = input("输入节点数量：")
consumer_node_num = 1
start_time = 0
ninety_percent = round(int(consumer_node_num) * 0.9)
seventy_percent = round(int(consumer_node_num) * 0.7)
last_receive_data_content = ""
data_propagation_info = {}
with open(data_file, "r", encoding="UTF-8") as file:
    for line in file:
        send_interest_match = re.match(send_interest_pattern, line)
        receive_interest_match = re.match(receive_interest_pattern, line)
        receive_data_match = re.match(receive_data_pattern, line)
        hop_count_match = re.match(hop_count_pattern, line)
        time_out_match = re.match(time_out_pattern, line)
        send_specific_interest_match = re.match(send_specific_interest_pattern, line)
        finish_match = re.match(finish_pattern, line)
        if send_interest_match:
            start_time = float(send_interest_match.group(1))

        elif send_specific_interest_match:
            time_stamp = float(send_specific_interest_match.group(1))
            send_node = int(send_specific_interest_match.group(2))
            send_for_data = str(send_specific_interest_match.group(3))
            if send_for_data not in send_interest_info:
                send_interest_info[send_for_data] = {send_node: time_stamp}
            else:
                send_interest_info[send_for_data][send_node] = time_stamp
            if send_node not in send_interest_node_info:
                send_interest_node_info[send_node] = {send_for_data: time_stamp}
            else:
                send_interest_node_info[send_node][send_for_data] = time_stamp

        elif receive_interest_match:
            time_stamp = float(receive_interest_match.group(1))
            receive_interest_data = str(receive_interest_match.group(3))
            if receive_interest_data in send_interest_info.keys():
                get_interest_info[receive_interest_data] = time_stamp
                # for d in send_interest_info[receive_interest_data].keys():
                #     #print(d)
        elif receive_data_match:
            time_stamp = float(receive_data_match.group(1))
            receive_node = int(receive_data_match.group(2))
            receive_data_content = str(receive_data_match.group(3))
            if (
                receive_data_content in send_interest_info.keys()
                and receive_data_content in get_interest_info.keys()
            ):
                if receive_node not in send_interest_info[receive_data_content].keys():
                    continue
                time = send_interest_info[receive_data_content][receive_node]
                all_time = time_stamp - time
                half_time = time_stamp - get_interest_info[receive_data_content]
                time_info.append(
                    f"Node {receive_node} get the data for {receive_data_content} in {all_time}s, from producer to "
                    f"consumer takes {half_time}s."
                )
                summary_info = {
                    "Send interest time": time,
                    "Data generate time": get_interest_info[receive_data_content],
                    "All time": all_time,
                    "Half time": half_time,
                }
                if receive_data_content not in summary_dict.keys():
                    summary_dict[receive_data_content] = {}
                summary_dict[receive_data_content][receive_node] = summary_info
                last_receive_data_content = receive_data_content

                send_interest_info[receive_data_content].pop(receive_node)
            if (
                receive_node in send_interest_node_info.keys()
                and receive_data_content in send_interest_node_info[receive_node].keys()
            ):
                time = send_interest_node_info[receive_node][receive_data_content]
                all_time = time_stamp - time
                node_time_info.append(
                    f"Node {receive_node} send interest for {receive_data_content} in {all_time}s"
                )
                node_summary_info = {
                    "Send interest time": time,
                    "Get Data time": time_stamp,
                    "All time": all_time,
                }
                if receive_node not in node_summary_dict.keys():
                    node_summary_dict[receive_node] = {}
                node_summary_dict[receive_node][
                    receive_data_content
                ] = node_summary_info
                last_receive_data_content = receive_data_content
                send_interest_node_info[receive_node].pop(last_receive_data_content)
        elif finish_match:
            node = int(finish_match.group(2))
            flatter(node)

        elif hop_count_match:
            hop_count = hop_count_match.group(3)
            last = time_info[-1]
            node = int(hop_count_match.group(2))
            time_info[-1] = last + f"hop count: {hop_count}"
            summary_dict[last_receive_data_content][node]["Hop count"] = hop_count
        elif time_out_match:
            pass
for item in time_info:
    print(item)


for data_name, node_data in summary_dict.items():
    all_nodes_time = 0
    all_nodes_half_time = 0
    for _, info in node_data.items():
        all_nodes_time += info["All time"]
        all_nodes_half_time += info["Half time"]
    all_nodes_time /= len(node_data)
    all_nodes_half_time /= len(node_data)
    if data_name in data_propagation_info.keys():
        data_propagation_info[data_name]["average all time"] = all_nodes_time
        data_propagation_info[data_name]["average half time"] = all_nodes_half_time


# for data_name, node_data in summary_dict.items():
#     for node_id, info in node_data.items():
#         row = {'data name': data_name,
#                'node id': node_id,
#                'Send interest time': info['Send interest time'],
#                'Data generate time': info['Data generate time'],
#                'All time': info['All time'],
#                'Half time': info['Half time'],
#                }
#         flattered_data.append(row)
#
# for data_name, time_data in data_propagation_info.items():
#     time_seventy = ''
#     time_ninety = ''
#
#     row = {'data name': data_name,
#            'average all time':time_data['average all time'],
#            'average half time':time_data['average half time']
#            }
#     flattered_propagation_data.append(row)
summary_average = {
    "node id": "average",
    "Start time": "-",
    "End time": "-",
    "All time": "-",
    "Start height": "-",
    "End height": "-",
    "Data num": "-",
    "95% time": sum(average_data) / len(average_data),
}
flattered_node_summary.append(summary_average)
df1 = pd.DataFrame(flattered_data)
df2 = pd.DataFrame(flattered_node_data)
df3 = pd.DataFrame(flattered_node_summary)

headers = [
    "data name",
    "node id",
    "Send interest time",
    "Data generate time",
    "All time",
    "Half time",
    "Hop count",
]
with pd.ExcelWriter(output_file, engine="xlsxwriter") as writer:
    df1.to_excel(writer, sheet_name="Detail Data", index=False)
    df2.to_excel(writer, sheet_name="Data Time", index=False)
    df3.to_excel(writer, sheet_name="summary", index=False)
