import json


tmp_receive = []
tmp_send = []

for i in range(1,100):
  time_dict_receive = {}
  time_dict_send = {}
  print (i)
  file1 = open('nt_' + str(i) +  '.n.json', 'r')
  lines = file1.readlines()
   

  total_messages_receive = 0
  total_messages_send = 0

  trace=lines[0]

  final_dictionary = json.loads(trace)
  for item in final_dictionary:
    timestamp = item[0]
    time_int = int(timestamp)
    m_type = item[1]
    if (m_type == 'r'):
      total_messages_receive += 1
      content = item[2]
      if time_int not in time_dict_receive:
        time_dict_receive[time_int] = 0
      time_dict_receive[time_int] += content[4]
    if (m_type == 's'):
      total_messages_send += 1
      content = item[2]
      if time_int not in time_dict_send:
        time_dict_send[time_int] = 0
      time_dict_send[time_int] += content[4]

  time_size_receive = time_dict_receive.values()
  time_size_send = time_dict_send.values()
  fls_tput_receive = [ x/1000/1000*8 for x in time_size_receive]
  fls_tput_send = [ x/1000/1000*8 for x in time_size_send]
  fls_tput_receive.sort()
  fls_tput_send.sort()
  print (fls_tput_receive[-1], fls_tput_send[-1])
  print (total_messages_receive, total_messages_send)
