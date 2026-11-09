#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

using namespace std;

bool downloadFile(const string& url, const string& savePath) {
	HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), savePath.c_str(), 0, NULL);
	return hr == S_OK;
}

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

    if (mode == "url") {
        localRepoPath = "data/packages.gz";

        cout << "\nDownloading from URL...\n";
        if (!downloadFile(repoUrl, localRepoPath)) {
            cerr << "Download failed!\n";
            return 1;
        }
        cout << "Downloaded successfully to " << localRepoPath << "\n";
    }
    else {
        cout << "\nUsing local file...\n";
    }

    cout << "\n=== Direct dependencies for " << packageName << " ===\n";
    vector<string> deps = parseDependencies(localRepoPath, packageName, version);

    if (deps.empty()) {
        cout << "No dependencies found (or package/version not found).\n";
    }
    else {
        for (const string& dep : deps) {
            cout << "  - " << dep << "\n";
        }
    }

	return 0;
}
