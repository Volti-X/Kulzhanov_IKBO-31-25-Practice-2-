#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <queue>
#include <set>
#include <map>

using namespace std;

string cleanPackageName(const string& dep) {
	size_t spacePos = dep.find(' ');
	if (spacePos != string::npos) {
		return dep.substr(0, spacePos);
	}
	return dep;
}

vector<string> parseDependencies(const string& filepath, const string& targetPackage, const string& targetVersion) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filepath << "\n";
        return {};
    }

    string line;
    bool inTargetPackage = false;
    bool versionMatched = false;
    vector<string> dependencies;

    while (getline(file, line)) {
        if (line.find("Package: ") == 0) {
            if (inTargetPackage && !versionMatched) inTargetPackage = false;

            if (line == "Package: " + targetPackage) {
                inTargetPackage = true;
                versionMatched = false;
            }
            else {
                inTargetPackage = false;
            }
        }
        else if (inTargetPackage && line.find("Version: ") == 0) {
            string currentVersion = line.substr(9);
            if (targetVersion == "latest" || currentVersion.find(targetVersion) == 0) {
                versionMatched = true;
            }
            else {
                inTargetPackage = false;
            }
        }
        else if (inTargetPackage && versionMatched && line.find("Depends: ") == 0) {
            string depsStr = line.substr(9);
            stringstream ss(depsStr);
            string dep;

            while (getline(ss, dep, ',')) {
                size_t start = dep.find_first_not_of(" ");
                if (start != string::npos) {
                    dep = dep.substr(start);
                }
                dependencies.push_back(cleanPackageName(dep));
            }
            break;
        }
    }
    return dependencies;
}

int main(int argc, char* argv[])
{
	string packageName = "";
	string repoUrl = "";
	string mode = "url";
	string version = "latest";
	string outputFile = "graph.png";
	string filter = "";

	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		if (arg == "--package" && i + 1 < argc) packageName = argv[++i];
		else if (arg == "--repo" && i + 1 < argc) repoUrl = argv[++i];
		else if (arg == "--mode" && i + 1 < argc) mode = argv[++i];
		else if (arg == "--version" && i + 1 < argc) version = argv[++i];
		else if (arg == "--output" && i + 1 < argc) outputFile = argv[++i];
		else if (arg == "--filter" && i + 1 < argc) filter = argv[++i];
	}

	cout << "=== Configuration ===" << "\n";
	cout << "package: " << (packageName.empty() ? "not set" : packageName) << "\n";
	cout << "repo: " << (repoUrl.empty() ? "not set" : repoUrl) << "\n";
	cout << "mode: " << mode << "\n";
	cout << "version: " << version << "\n";
	cout << "output: " << outputFile << "\n";
	cout << "filter: " << (filter.empty() ? "not set" : filter) << "\n";
	cout << "=====================" << "\n";

	if (packageName.empty()) {
		cerr << "Error: --package is required!\n";
		return 1;
	}
	if (repoUrl.empty()) {
		cerr << "Error: --repo is required!\n";
		return 1;
	}

    string localRepoPath = repoUrl;

    cout << "\n=== Building dependency graph (BFS) ===\n";

    queue<string> q;
    set<string> visited;
    map<string, vector<string>> graph;

    q.push(packageName);

    while (!q.empty()) {
        string current = q.front();
        q.pop();

        if (visited.count(current))
            continue;
        visited.insert(current);

        if (!filter.empty() && current.find(filter) != string::npos) {
            cout << "[Filtered out]: " << current << "\n";
            continue;
        }

        vector<string> deps = parseDependencies(localRepoPath, current, "latest");
        graph[current] = deps;

        for (const string& dep : deps) {
            if (!visited.count(dep)) {
                q.push(dep);
            }
        }
    }

    if (!filter.empty()) {
        for (auto& pair : graph) {
            vector<string> cleanDeps;
            for (const string& dep : pair.second) {
                if (dep.find(filter) == string::npos) {
                    cleanDeps.push_back(dep);
                }
            }
            pair.second = cleanDeps;
        }
    }

    cout << "\n=== Resulting Dependency Graph ===\n";
    for (auto const& pair : graph) {
        cout << pair.first << " -> ";
        if (pair.second.empty()) {
            cout << "(no dependencies)\n";
        }
        else {
            for (size_t i = 0; i < pair.second.size(); ++i) {
                cout << pair.second[i];
                if (i < pair.second.size() - 1) cout << ", ";
            }
            cout << "\n";
        }
    }

    cout << "\n=== Direct dependencies for " << packageName << " ===\n";
    vector<string> deps = parseDependencies(localRepoPath, packageName, version);

    vector<string> filteredDeps;
    for (const string& dep : deps) {
        if (filter.empty() || dep.find(filter) == string::npos)
            filteredDeps.push_back(dep);
    }
    if (filteredDeps.empty()) {
        cout << "No dependencies found (or package/version not found).\n";
    }
    else {
        for (const string& dep : filteredDeps) {
            cout << "  - " << dep << "\n";
        }
    }

    cout << "\n=== Reverse dependencies for " << packageName << " ===\n";
    bool found = false;

    for (const auto& pair : graph) {
        for (const string& dep : pair.second) {
            if (dep == packageName) {
                cout << "  - " << pair.first << "\n";
                found = true;
            }
        }
    }

    if (!found) {
        cout << "No reverse dependencies found.\n";
    }

	return 0;
}
