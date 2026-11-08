#include <iostream>
#include <string>
using namespace std;

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

	return 0;
}
