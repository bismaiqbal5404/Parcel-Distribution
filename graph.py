import networkx as nx
import matplotlib.pyplot as plt
import math
import random

# -----------------------------------------
# 1. Parse `routes.txt` into a NetworkX graph
# -----------------------------------------
G = nx.Graph()
node_zones = {}   # node_code → zone_id
zone_names = {}   # zone_id → human-readable name

current_zone = None
with open('routes.txt', 'r') as f:
    for raw in f:
        line = raw.strip()
        if not line or line.startswith('#'):
            continue

        parts = line.split(maxsplit=2)
        keyword = parts[0]

        if keyword == 'ZONE':
            zone_id = parts[1]
            zone_name = parts[2].strip().strip('"')
            current_zone = zone_id
            zone_names[zone_id] = zone_name

        elif keyword == 'NODE':
            node_code = parts[1]
            label = parts[2].strip().strip('"')
            G.add_node(node_code, label=label, zone=current_zone)
            node_zones[node_code] = current_zone

        elif keyword == 'EDGE':
            u = parts[1]
            rest = parts[2].split()
            v = rest[0]
            weight = float(rest[1])
            G.add_edge(u, v, weight=weight)

# -----------------------------------------
# 2. Group nodes by zone
# -----------------------------------------
zones = sorted(zone_names.keys())  # e.g. ['A', 'B', 'C', ...]
zone_subgraphs = {}               # store each zone’s induced subgraph
for z in zones:
    members = [n for n, data in G.nodes(data=True) if data['zone'] == z]
    subG = G.subgraph(members).copy()
    zone_subgraphs[z] = subG

# -----------------------------------------
# 3. Compute “big circle” centers for each zone
# -----------------------------------------
num_zones = len(zones)
overall_radius = 15    # give more room overall
zone_centers = {}
for i, z in enumerate(zones):
    angle = 2 * math.pi * i / num_zones
    cx = overall_radius * math.cos(angle)
    cy = overall_radius * math.sin(angle)
    zone_centers[z] = (cx, cy)

# -----------------------------------------
# 4. For each zone, compute a local layout
#    (we’ll use a circular layout here; a tiny spring-layout is also possible)
# -----------------------------------------
pos = {}
local_scale = 4.0     # radius of each zone’s local layout circle
for z in zones:
    subG = zone_subgraphs[z]
    members = list(subG.nodes())
    n_count = len(members)

    if n_count == 0:
        continue

    # Option A: circular layout within the zone
    # =========================================
    # This simply places each node evenly around a circle of radius local_scale.
    # You can add jitter to spread them out a bit more.
    for idx, node in enumerate(members):
        angle = 2 * math.pi * idx / n_count
        r = local_scale + random.uniform(-0.5, 0.5)  # small jitter
        x_loc = r * math.cos(angle)
        y_loc = r * math.sin(angle)

        # translate from local (x_loc, y_loc) to the zone’s global center
        center_x, center_y = zone_centers[z]
        pos[node] = (center_x + x_loc, center_y + y_loc)

    # Option B: small spring layout within each zone (uncomment to try)
    # ==================================================================
    # temp_pos = nx.spring_layout(subG, seed=42, k=0.3, iterations=50)
    # for node, (lx, ly) in temp_pos.items():
    #     # normalize temp positions to lie roughly on a circle of radius local_scale
    #     norm = math.sqrt(lx*lx + ly*ly)
    #     if norm == 0:
    #         dx, dy = local_scale, 0
    #     else:
    #         dx = (lx / norm) * (local_scale + random.uniform(-0.5,0.5))
    #         dy = (ly / norm) * (local_scale + random.uniform(-0.5,0.5))
    #     cx, cy = zone_centers[z]
    #     pos[node] = (cx + dx, cy + dy)

# -----------------------------------------
# 5. Color each node by zone
# -----------------------------------------
cmap = plt.get_cmap('tab20')
zone_color_map = {zone: cmap(i) for i, zone in enumerate(zones)}
node_colors = [zone_color_map[node_zones[n]] for n in G.nodes()]

# -----------------------------------------
# 6. Draw everything with larger fonts and sizes
# -----------------------------------------
plt.figure(figsize=(20, 20))

# Separate intra-zone vs inter-zone edges
intra_edges = [(u, v) for u, v in G.edges() if node_zones[u] == node_zones[v]]
inter_edges = [(u, v) for u, v in G.edges() if node_zones[u] != node_zones[v]]

# Draw nodes
nx.draw_networkx_nodes(
    G,
    pos,
    node_color=node_colors,
    node_size=400,
    alpha=0.9
)

# Draw edges: intra-zone thick & light, inter-zone thin & dashed
nx.draw_networkx_edges(
    G, pos,
    edgelist=intra_edges,
    width=3,
    alpha=0.7,
    edge_color='gray'
)
nx.draw_networkx_edges(
    G, pos,
    edgelist=inter_edges,
    width=1.5,
    alpha=0.6,
    edge_color='black',
    style='dashed'
)

# Draw node labels (code only; bigger font)
nx.draw_networkx_labels(
    G,
    pos,
    labels={n: n for n in G.nodes()},
    font_size=10,
    font_weight='bold'
)

# Draw edge-weight labels (larger font)
edge_labels = nx.get_edge_attributes(G, 'weight')
nx.draw_networkx_edge_labels(
    G,
    pos,
    edge_labels=edge_labels,
    font_size=8,
    label_pos=0.5
)

# -----------------------------------------
# 7. Legend of zones (zone-color mapping)
# -----------------------------------------
for zone, color in zone_color_map.items():
    plt.scatter([], [], c=[color], label=f"{zone}: {zone_names[zone]}", s=150)

plt.legend(
    scatterpoints=1,
    frameon=False,
    fontsize=12,
    bbox_to_anchor=(1.05, 1),
    loc='upper left'
)

plt.title("Zone-based Parcel Delivery Graph (Expanded Layout)", fontsize=18)
plt.axis('off')
plt.tight_layout()
plt.show()
