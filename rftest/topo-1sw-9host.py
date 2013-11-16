""" rftest4 topology

authors: Davi G. Peterlini (davip@cpqd.com.br)

One switches connected in mesh topology plus a host for each switch:

       h1 --- sT ---- h2

"""

from mininet.topo import Topo

class rftest4(Topo):
    "RouteFlow Demo Setup"

    def __init__( self, enable_all = True ):
        "Create custom topo."

        Topo.__init__( self )

        switch = self.addSwitch("s1")

        for h in range():
            host = self.addHost('h%s' % (h + 1), 
                                ip="172.31.%s.100/24" % (h + 1),
                                defaultRoute="gw 172.31.%s.1" % (h + 1))
            
            self.addLink(host, switch)

topos = { 'rftest4': ( lambda: rftest4() ) }