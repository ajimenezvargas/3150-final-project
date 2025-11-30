// Preset data for showcase simulations
const PRESET_SIMULATIONS = [
    {
        name: "Tech Giants Network",
        description: "Simulates BGP routing between major tech companies",
        caida: `# AS Relationships (asn1|asn2|relationship: -1=customer, 0=peer, 1=provider)
15169|174|0
15169|3356|0
15169|6453|0
8452|15169|-1
8452|174|0
8452|3356|0
1239|174|1
1239|3356|0
6939|174|0
6939|3356|0
701|174|-1
701|3356|-1
3320|174|1
3320|3356|1
13335|174|0
13335|3356|0
13335|15169|0
6762|174|0
6762|3356|0
12389|15169|-1
12389|174|0
12389|3356|0`,
        announcements: `origin_asn,prefix,rov_invalid
15169,8.8.8.0/24,false
15169,8.8.4.0/24,false
13335,1.1.1.0/24,false
13335,1.0.0.0/24,false
174,38.0.0.0/8,false
3356,67.0.0.0/8,false
701,206.0.0.0/8,false`,
        rov: `asn
15169
13335`,
        targetAsns: [174, 3356, 701]
    },
    {
        name: "Academic Network",
        description: "Simulates BGP routing across university networks",
        caida: `# AS Relationships (asn1|asn2|relationship: -1=customer, 0=peer, 1=provider)
1|2|1
2|3|0
2|4|0
3|4|0
5|2|-1
5|4|-1
6|1|-1
6|3|0
7|4|-1
8|2|-1
9|3|-1`,
        announcements: `origin_asn,prefix,rov_invalid
1,192.168.1.0/24,false
2,192.168.2.0/24,false
3,192.168.3.0/24,false
4,192.168.4.0/24,false
5,192.168.5.0/24,false
6,192.168.6.0/24,false`,
        rov: `asn
1
3`,
        targetAsns: [2, 4, 5]
    },
    {
        name: "Regional ISP Network",
        description: "Simulates routing in a regional internet service provider",
        caida: `# AS Relationships (asn1|asn2|relationship: -1=customer, 0=peer, 1=provider)
64500|64501|-1
64500|64502|-1
64501|64502|0
64502|64503|1
64502|64504|1
64505|64501|-1
64505|64502|0
64506|64500|-1
64506|64501|0`,
        announcements: `origin_asn,prefix,rov_invalid
64500,10.0.0.0/8,false
64501,172.16.0.0/12,false
64502,192.0.2.0/24,false
64503,198.51.100.0/24,false
64504,203.0.113.0/24,false`,
        rov: `asn
64500
64502`,
        targetAsns: [64501, 64502]
    }
];

function getRandomSimulation() {
    return PRESET_SIMULATIONS[Math.floor(Math.random() * PRESET_SIMULATIONS.length)];
}

function getRandomTargetAsn(simulation) {
    return simulation.targetAsns[Math.floor(Math.random() * simulation.targetAsns.length)];
}
