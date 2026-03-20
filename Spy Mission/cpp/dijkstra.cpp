#include <queue>
#include <algorithm>
#include "gdata.h"
#include "dijkstra.h"

using namespace std;

vector<int> dijkstra(int start, int end, bool blocked[]) {
    vector<int> dist(n, 1e9);
    vector<int> parent(n, -1);

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

    dist[start] = 0;
    pq.push({0,start});

    while(!pq.empty()) {
        int node = pq.top().second;
        int cost = pq.top().first;
        pq.pop();

        for(auto edge : graph[node]) {
            if(blocked[edge.dest_node]) continue;

            if(dist[edge.dest_node] > cost + edge.weight) {
                dist[edge.dest_node] = cost + edge.weight;
                parent[edge.dest_node] = node;
                pq.push({dist[edge.dest_node], edge.dest_node});
            }
        }
    }

    vector<int> path;

    for(int v=end; v!=-1; v=parent[v])
        path.push_back(v);

    reverse(path.begin(), path.end());

    return path;
}