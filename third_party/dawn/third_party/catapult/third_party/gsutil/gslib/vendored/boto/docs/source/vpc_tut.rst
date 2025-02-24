.. _vpc_tut:

=======================================
An Introduction to boto's VPC interface
=======================================

This tutorial is based on the examples in the Amazon Virtual Private
Cloud Getting Started Guide (http://docs.amazonwebservices.com/AmazonVPC/latest/GettingStartedGuide/).
In each example, it tries to show the boto request that correspond to
the AWS command line tools.

Creating a VPC connection
-------------------------
First, we need to create a new VPC connection:

>>> from boto.vpc import VPCConnection
>>> c = VPCConnection()

To create a VPC
---------------
Now that we have a VPC connection, we can create our first VPC.

>>> vpc = c.create_vpc('10.0.0.0/24')
>>> vpc
VPC:vpc-6b1fe402
>>> vpc.id
u'vpc-6b1fe402'
>>> vpc.state
u'pending'
>>> vpc.cidr_block
u'10.0.0.0/24'
>>> vpc.dhcp_options_id
u'default'
>>> 

To create a subnet
------------------
The next step is to create a subnet to associate with your VPC.

>>> subnet = c.create_subnet(vpc.id, '10.0.0.0/25')
>>> subnet.id
u'subnet-6a1fe403'
>>> subnet.state
u'pending'
>>> subnet.cidr_block
u'10.0.0.0/25'
>>> subnet.available_ip_address_count
123
>>> subnet.availability_zone
u'us-east-1b'
>>> 

To create a customer gateway
----------------------------
Next, we create a customer gateway.

>>> cg = c.create_customer_gateway('ipsec.1', '12.1.2.3', 65534)
>>> cg.id
u'cgw-b6a247df'
>>> cg.type
u'ipsec.1'
>>> cg.state
u'available'
>>> cg.ip_address
u'12.1.2.3'
>>> cg.bgp_asn
u'65534'
>>> 

To create a VPN gateway
-----------------------

>>> vg = c.create_vpn_gateway('ipsec.1')
>>> vg.id
u'vgw-44ad482d'
>>> vg.type
u'ipsec.1'
>>> vg.state
u'pending'
>>> vg.availability_zone
u'us-east-1b'
>>>

Attaching a VPN Gateway to a VPC
--------------------------------

>>> vg.attach(vpc.id)
>>>

Associating an Elastic IP with a VPC Instance
---------------------------------------------

>>> ec2.connection.associate_address('i-71b2f60b', None, 'eipalloc-35cf685d')
>>>

Releasing an Elastic IP Attached to a VPC Instance
--------------------------------------------------

>>> ec2.connection.release_address(None, 'eipalloc-35cf685d')
>>>

To Get All VPN Connections
--------------------------
>>> vpns = c.get_all_vpn_connections()
>>> vpns[0].id
u'vpn-12ef67bv'
>>> tunnels = vpns[0].tunnels
>>> tunnels
[VpnTunnel: 177.12.34.56, VpnTunnel: 177.12.34.57]


To Create VPC Peering Connection
--------------------------------
>>> vpcs = c.get_all_vpcs()
>>> vpc_peering_connection = c.create_vpc_peering_connection(vpcs[0].id, vpcs[1].id)
>>> vpc_peering_connection
VpcPeeringConnection:pcx-18987471

To Accept VPC Peering Connection
--------------------------------
>>> vpc_peering_connections = c.get_all_vpc_peering_connections()
>>> vpc_peering_connection = vpc_peering_connections[0]
>>> vpc_peering_connection.status_code
u'pending-acceptance'
>>> vpc_peering_connection = c.accept_vpc_peering_connection(vpc_peering_connection.id)
>>> vpc_peering_connection.update()
u'active'

To Reject VPC Peering Connection
--------------------------------
>>> vpc_peering_connections = c.get_all_vpc_peering_connections()
>>> vpc_peering_connection = vpc_peering_connections[0]
>>> vpc_peering_connection.status_code
u'pending-acceptance
>>> c.reject_vpc_peering_connection(vpc_peering_connection.id)
>>> vpc_peering_connection.update()
u'rejected'

