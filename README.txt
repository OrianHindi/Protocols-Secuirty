How to run the program?

compile commad:

make - will compile 2 exec file,those file will be named ipv4_flood and ipv6_flood.

make ipv4- will complie only 1 exec file, this file name will be ipv4_flood.

make ipv6- will compile only 1 exec file, this file name will be ipv6_flood.


exec commad:

ipv4_flood:
this exec will generate a flood attack, TCP or UDP by the user choise.
this exec have few options:

./ipv4_flood -t ipv4_address -p port -r --  
**all of this options are optional.(order doesnt matter)**

-t means that user want to change the deafault ipv4 address to  another target ipv4 address

-p means that user want to change the deafault port to choosen port.

-r means change to default attack (TCP RST FLOOD ATTACK) to a UDP flood attack.


./ipv4_flood will generate a default TCP RST attack to the default ipv4 address and port. 


ipv6_flood:
this exec will generate an ipv6 UDP flood attack.
this exec have few options:

./ipv6_flood -t tartget_ipv6_address -p target_port
**all of this options are optional(order doesnt matter)**

same as ipv4_flood:
 -t means user want to change the target_ipv6_address.
 -p means user want to change the taget_port.
 
 
 clean command:
 
 make clean will delete *.o files and the exec files.
