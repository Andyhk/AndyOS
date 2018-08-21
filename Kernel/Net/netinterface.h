#pragma once
#include "types.h"
#include "PCI/pci.h"
#include "netpacket.h"
#include "address.h"

class NetInterface /*: public PciDevice*/
{
public:
	IPv4Address gateway_addr;

	NetInterface(PciDevice* pci_dev);

	virtual void Send(NetPacket* pkt) { }
	virtual void Poll() { }

	//static void(*EthReceive)(NetInterface* intf, NetPacket* pkt);

	virtual MacAddress GetMac() { return MacAddress(); }
	virtual IPv4Address GetIP() { return IPv4Address(); }

	PciDevice* dev;
};