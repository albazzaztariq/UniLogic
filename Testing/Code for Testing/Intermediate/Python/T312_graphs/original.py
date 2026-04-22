"""Graph algorithms — adjacency matrix, BFS, DFS,
shortest paths, connected components, cycle detection, topological sort."""


def graph_init(adj: list, size: int) -> None:
    i: int = 0
    while i < size * size:
        adj[i] = 0
        i = i + 1


def graph_add_edge(adj: list, size: int, u: int, v: int) -> None:
    adj[u * size + v] = 1
    adj[v * size + u] = 1


def graph_add_directed(adj: list, size: int, u: int, v: int) -> None:
    adj[u * size + v] = 1


def graph_has_edge(adj: list, size: int, u: int, v: int) -> bool:
    if adj[u * size + v] == 1:
        return True
    return False


def graph_degree(adj: list, size: int, node: int) -> int:
    count: int = 0
    i: int = 0
    while i < size:
        if adj[node * size + i] == 1:
            count = count + 1
        i = i + 1
    return count


def graph_edge_count(adj: list, size: int) -> int:
    count: int = 0
    i: int = 0
    while i < size:
        j: int = i + 1
        while j < size:
            if adj[i * size + j] == 1:
                count = count + 1
            j = j + 1
        i = i + 1
    return count


def bfs(adj: list, size: int, start: int, visited: list, order: list) -> int:
    queue: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    head: int = 0
    tail: int = 0
    count: int = 0
    i: int = 0
    while i < size:
        visited[i] = 0
        i = i + 1
    queue[tail] = start
    tail = tail + 1
    visited[start] = 1
    while head < tail:
        node: int = queue[head]
        head = head + 1
        order[count] = node
        count = count + 1
        neighbor: int = 0
        while neighbor < size:
            if adj[node * size + neighbor] == 1 and visited[neighbor] == 0:
                visited[neighbor] = 1
                queue[tail] = neighbor
                tail = tail + 1
            neighbor = neighbor + 1
    return count


def dfs_helper(adj: list, size: int, node: int, visited: list, order: list, count: list) -> None:
    visited[node] = 1
    order[count[0]] = node
    count[0] = count[0] + 1
    neighbor: int = 0
    while neighbor < size:
        if adj[node * size + neighbor] == 1 and visited[neighbor] == 0:
            dfs_helper(adj, size, neighbor, visited, order, count)
        neighbor = neighbor + 1


def dfs(adj: list, size: int, start: int, visited: list, order: list) -> int:
    i: int = 0
    while i < size:
        visited[i] = 0
        i = i + 1
    count: list = [0]
    dfs_helper(adj, size, start, visited, order, count)
    return count[0]


def count_components(adj: list, size: int) -> int:
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    components: int = 0
    order: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    count_arr: list = [0]
    i: int = 0
    while i < size:
        if visited[i] == 0:
            count_arr[0] = 0
            dfs_helper(adj, size, i, visited, order, count_arr)
            components = components + 1
        i = i + 1
    return components


def shortest_path_bfs(adj: list, size: int, start: int, end: int) -> int:
    if start == end:
        return 0
    dist: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    queue: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    head: int = 0
    tail: int = 0
    i: int = 0
    while i < size:
        dist[i] = -1
        i = i + 1
    dist[start] = 0
    visited[start] = 1
    queue[tail] = start
    tail = tail + 1
    while head < tail:
        node: int = queue[head]
        head = head + 1
        neighbor: int = 0
        while neighbor < size:
            if adj[node * size + neighbor] == 1 and visited[neighbor] == 0:
                visited[neighbor] = 1
                dist[neighbor] = dist[node] + 1
                if neighbor == end:
                    return dist[neighbor]
                queue[tail] = neighbor
                tail = tail + 1
            neighbor = neighbor + 1
    return -1


def has_cycle_helper(adj: list, size: int, node: int, parent: int, visited: list) -> bool:
    visited[node] = 1
    neighbor: int = 0
    while neighbor < size:
        if adj[node * size + neighbor] == 1:
            if visited[neighbor] == 0:
                if has_cycle_helper(adj, size, neighbor, node, visited):
                    return True
            else:
                if neighbor != parent:
                    return True
        neighbor = neighbor + 1
    return False


def has_cycle(adj: list, size: int) -> bool:
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    i: int = 0
    while i < size:
        if visited[i] == 0:
            if has_cycle_helper(adj, size, i, -1, visited):
                return True
        i = i + 1
    return False


def is_bipartite(adj: list, size: int) -> bool:
    color: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    queue: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    i: int = 0
    while i < size:
        if visited[i] == 0:
            visited[i] = 1
            color[i] = 1
            head: int = 0
            tail: int = 0
            queue[tail] = i
            tail = tail + 1
            while head < tail:
                node: int = queue[head]
                head = head + 1
                neighbor: int = 0
                while neighbor < size:
                    if adj[node * size + neighbor] == 1:
                        if visited[neighbor] == 0:
                            visited[neighbor] = 1
                            if color[node] == 1:
                                color[neighbor] = 2
                            else:
                                color[neighbor] = 1
                            queue[tail] = neighbor
                            tail = tail + 1
                        else:
                            if color[neighbor] == color[node]:
                                return False
                    neighbor = neighbor + 1
        i = i + 1
    return True


def topo_sort_helper(adj: list, size: int, node: int, visited: list, stack: list, stack_top: list) -> None:
    visited[node] = 1
    neighbor: int = 0
    while neighbor < size:
        if adj[node * size + neighbor] == 1 and visited[neighbor] == 0:
            topo_sort_helper(adj, size, neighbor, visited, stack, stack_top)
        neighbor = neighbor + 1
    stack[stack_top[0]] = node
    stack_top[0] = stack_top[0] + 1


def topo_sort(adj: list, size: int, result: list) -> int:
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    stack: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    stack_top: list = [0]
    i: int = 0
    while i < size:
        if visited[i] == 0:
            topo_sort_helper(adj, size, i, visited, stack, stack_top)
        i = i + 1
    count: int = stack_top[0]
    i = 0
    while i < count:
        result[i] = stack[count - 1 - i]
        i = i + 1
    return count


def floyd_warshall(adj: list, size: int, dist: list) -> None:
    inf: int = 99999
    i: int = 0
    while i < size:
        j: int = 0
        while j < size:
            if i == j:
                dist[i * size + j] = 0
            else:
                if adj[i * size + j] == 1:
                    dist[i * size + j] = 1
                else:
                    dist[i * size + j] = inf
            j = j + 1
        i = i + 1
    k: int = 0
    while k < size:
        i = 0
        while i < size:
            j = 0
            while j < size:
                new_dist: int = dist[i * size + k] + dist[k * size + j]
                if new_dist < dist[i * size + j]:
                    dist[i * size + j] = new_dist
                j = j + 1
            i = i + 1
        k = k + 1


def print_array(arr: list, count: int) -> None:
    result: str = ""
    i: int = 0
    while i < count:
        if i > 0:
            result = result + " "
        result = result + str(arr[i])
        i = i + 1
    print(result)


def print_matrix(mat: list, rows: int, cols: int) -> None:
    r: int = 0
    while r < rows:
        line: str = ""
        c: int = 0
        while c < cols:
            if c > 0:
                line = line + " "
            val: int = mat[r * cols + c]
            if val >= 99999:
                line = line + "INF"
            else:
                line = line + str(val)
            c = c + 1
        print(line)
        r = r + 1


def test_basic_graph() -> None:
    print("=== Basic Graph ===")
    size: int = 6
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_edge(adj, size, 0, 1)
    graph_add_edge(adj, size, 0, 2)
    graph_add_edge(adj, size, 1, 3)
    graph_add_edge(adj, size, 2, 3)
    graph_add_edge(adj, size, 3, 4)
    graph_add_edge(adj, size, 4, 5)
    print("Edges: " + str(graph_edge_count(adj, size)))
    i: int = 0
    while i < size:
        print("Degree " + str(i) + ": " + str(graph_degree(adj, size, i)))
        i = i + 1
    if graph_has_edge(adj, size, 0, 1):
        print("Edge 0-1: yes")
    if not graph_has_edge(adj, size, 0, 5):
        print("Edge 0-5: no")


def test_bfs() -> None:
    print("=== BFS ===")
    size: int = 7
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_edge(adj, size, 0, 1)
    graph_add_edge(adj, size, 0, 2)
    graph_add_edge(adj, size, 1, 3)
    graph_add_edge(adj, size, 1, 4)
    graph_add_edge(adj, size, 2, 5)
    graph_add_edge(adj, size, 2, 6)
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    order: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    count: int = bfs(adj, size, 0, visited, order)
    print("BFS from 0:")
    print_array(order, count)
    print("Visited: " + str(count))


def test_dfs() -> None:
    print("=== DFS ===")
    size: int = 7
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_edge(adj, size, 0, 1)
    graph_add_edge(adj, size, 0, 2)
    graph_add_edge(adj, size, 1, 3)
    graph_add_edge(adj, size, 1, 4)
    graph_add_edge(adj, size, 2, 5)
    graph_add_edge(adj, size, 2, 6)
    visited: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    order: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    count: int = dfs(adj, size, 0, visited, order)
    print("DFS from 0:")
    print_array(order, count)
    print("Visited: " + str(count))


def test_shortest_path() -> None:
    print("=== Shortest Path ===")
    size: int = 6
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_edge(adj, size, 0, 1)
    graph_add_edge(adj, size, 1, 2)
    graph_add_edge(adj, size, 2, 3)
    graph_add_edge(adj, size, 3, 4)
    graph_add_edge(adj, size, 4, 5)
    graph_add_edge(adj, size, 0, 3)
    print("Dist 0->5: " + str(shortest_path_bfs(adj, size, 0, 5)))
    print("Dist 0->3: " + str(shortest_path_bfs(adj, size, 0, 3)))
    print("Dist 0->0: " + str(shortest_path_bfs(adj, size, 0, 0)))
    print("Dist 1->5: " + str(shortest_path_bfs(adj, size, 1, 5)))


def test_components() -> None:
    print("=== Connected Components ===")
    size: int = 8
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_edge(adj, size, 0, 1)
    graph_add_edge(adj, size, 1, 2)
    graph_add_edge(adj, size, 3, 4)
    graph_add_edge(adj, size, 5, 6)
    graph_add_edge(adj, size, 5, 7)
    print("Components: " + str(count_components(adj, size)))

    size2: int = 5
    adj2: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj2, size2)
    graph_add_edge(adj2, size2, 0, 1)
    graph_add_edge(adj2, size2, 1, 2)
    graph_add_edge(adj2, size2, 2, 3)
    graph_add_edge(adj2, size2, 3, 4)
    graph_add_edge(adj2, size2, 4, 0)
    print("Components full cycle: " + str(count_components(adj2, size2)))


def test_cycle() -> None:
    print("=== Cycle Detection ===")
    size: int = 5
    adj_tree: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj_tree, size)
    graph_add_edge(adj_tree, size, 0, 1)
    graph_add_edge(adj_tree, size, 1, 2)
    graph_add_edge(adj_tree, size, 2, 3)
    graph_add_edge(adj_tree, size, 3, 4)
    if has_cycle(adj_tree, size):
        print("Tree has cycle: yes")
    else:
        print("Tree has cycle: no")

    adj_cycle: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj_cycle, size)
    graph_add_edge(adj_cycle, size, 0, 1)
    graph_add_edge(adj_cycle, size, 1, 2)
    graph_add_edge(adj_cycle, size, 2, 3)
    graph_add_edge(adj_cycle, size, 3, 0)
    if has_cycle(adj_cycle, size):
        print("Cycle graph has cycle: yes")
    else:
        print("Cycle graph has cycle: no")


def test_bipartite() -> None:
    print("=== Bipartite ===")
    size: int = 6
    adj_bi: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj_bi, size)
    graph_add_edge(adj_bi, size, 0, 3)
    graph_add_edge(adj_bi, size, 0, 4)
    graph_add_edge(adj_bi, size, 1, 3)
    graph_add_edge(adj_bi, size, 1, 5)
    graph_add_edge(adj_bi, size, 2, 4)
    graph_add_edge(adj_bi, size, 2, 5)
    if is_bipartite(adj_bi, size):
        print("Bipartite graph: yes")
    else:
        print("Bipartite graph: no")

    adj_tri: list = [0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj_tri, 3)
    graph_add_edge(adj_tri, 3, 0, 1)
    graph_add_edge(adj_tri, 3, 1, 2)
    graph_add_edge(adj_tri, 3, 2, 0)
    if is_bipartite(adj_tri, 3):
        print("Triangle: bipartite")
    else:
        print("Triangle: not bipartite")


def test_topo_sort() -> None:
    print("=== Topological Sort ===")
    size: int = 6
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_directed(adj, size, 5, 2)
    graph_add_directed(adj, size, 5, 0)
    graph_add_directed(adj, size, 4, 0)
    graph_add_directed(adj, size, 4, 1)
    graph_add_directed(adj, size, 2, 3)
    graph_add_directed(adj, size, 3, 1)
    result: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    count: int = topo_sort(adj, size, result)
    print("Topo order:")
    print_array(result, count)


def test_floyd() -> None:
    print("=== Floyd-Warshall ===")
    size: int = 4
    adj: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    graph_init(adj, size)
    graph_add_directed(adj, size, 0, 1)
    graph_add_directed(adj, size, 1, 2)
    graph_add_directed(adj, size, 2, 3)
    graph_add_directed(adj, size, 0, 2)
    dist: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    floyd_warshall(adj, size, dist)
    print("Distance matrix:")
    print_matrix(dist, size, size)


def main() -> int:
    test_basic_graph()
    test_bfs()
    test_dfs()
    test_shortest_path()
    test_components()
    test_cycle()
    test_bipartite()
    test_topo_sort()
    test_floyd()
    return 0


if __name__ == "__main__":
    main()
