#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

// Store created tables and their data
unordered_set<string> createdTables;
unordered_map<string, vector<string>> tableData;
ofstream outputFile;  // Output file stream

// Function prototypes
void createFile(const string& filename);
void createTable(const string& line);
void insertIntoTable(const string& line);
void displayTables();
void processLine(const string& line, const string& filename);
void displayDatabasePath(const string& filename);
void writeToOutputFile(const string& message);

void writeToOutputFile(const string& message) {
    cout << message << endl;        // Display on screen
    if (outputFile.is_open()) {
        outputFile << message << endl;  // Write to file
    }
}

void createFile(const string& filename) {
    outputFile.open(filename);
    if (outputFile) {
        writeToOutputFile("File created: " + filename);
    } else {
        writeToOutputFile("Error creating file: " + filename);
    }
}

void createTable(const string& line) {
    size_t pos = line.find("TABLE") + 6;  // Get position after "CREATE TABLE "
    size_t endPos = line.find('(', pos); // Find position of '('
    string tableName = line.substr(pos, endPos - pos); // Extract table name
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    if (createdTables.find(tableName) == createdTables.end()) {
        createdTables.insert(tableName);
        writeToOutputFile("Table created: " + tableName);
    } else {
        writeToOutputFile("Table already exists: " + tableName);
    }
}

void insertIntoTable(const string& line) {
    size_t pos = line.find("INSERT INTO") + 11; // Position after "INSERT INTO "
    size_t tableEnd = line.find('(', pos);      // End of table name
    string tableName = line.substr(pos, tableEnd - pos);
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    if (createdTables.find(tableName) == createdTables.end()) {
        writeToOutputFile("Table does not exist: " + tableName);
        return;
    }

    size_t valuesPos = line.find("VALUES", tableEnd) + 6; // Position after "VALUES"
    string values = line.substr(valuesPos);
    values.erase(remove_if(values.begin(), values.end(), ::isspace), values.end());

    tableData[tableName].push_back(values);
    writeToOutputFile("Inserted into table " + tableName + ": " + values);
}

void displayTables() {
    if (createdTables.empty()) {
        writeToOutputFile("No tables have been created.");
    } else {
        writeToOutputFile("Tables:");
        for (const string& tableName : createdTables) {
            writeToOutputFile("- " + tableName);
        }
    }
}

void displayDatabasePath(const string& filename) {
    writeToOutputFile("Path of the .mdb file: " + filename);
}

void processLine(const string& line, const string& filename) {
    // Print each line
    writeToOutputFile(line);

    if (line.find("CREATE") != string::npos) {
        // Check if it's CREATE FILE or CREATE TABLE
        size_t pos = line.find("CREATE") + 6;
        size_t spacePos = line.find(' ', pos);
        string fileName = line.substr(spacePos + 1);
        fileName.erase(remove_if(fileName.begin(), fileName.end(), ::isspace), fileName.end());

        // If it's creating a file (not a table), just create the file
        if (line.find("TABLE") == string::npos) {
            createFile(fileName);
        }
        // Otherwise, create the table
        else {
            createTable(line);
        }
    } else if (line.find("DATABASES") != string::npos) {
        displayDatabasePath(filename);
    } else if (line.find("TABLES") != string::npos) {
        displayTables();
    } else if (line.find("INSERT INTO") != string::npos) {
        insertIntoTable(line);
    } else if (line.find("SELECT * FROM") != string::npos) {
        string tableName = line.substr(14, line.find(';') - 14);
        tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

        if (createdTables.find(tableName) == createdTables.end()) {
            writeToOutputFile("Table does not exist: " + tableName);
        } else {
            writeToOutputFile("Table: " + tableName);
            writeToOutputFile("Data (CSV format):");
            for (const string& row : tableData[tableName]) {
                writeToOutputFile(row);
            }
        }
    } else if (line.find("SELECT COUNT(*) FROM") != string::npos) {
        string tableName = line.substr(20, line.find(';') - 20);
        tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

        if (createdTables.find(tableName) == createdTables.end()) {
            writeToOutputFile("Table does not exist: " + tableName);
        } else {
            size_t rowCount = tableData[tableName].size();
            writeToOutputFile("Row count for table " + tableName + ": " + to_string(rowCount));
        }
    }
}

int main() {
    string basePath = "C:\\Users\\User\\Projects\\Light_Mariadb_Interpreter\\data\\";
    string fileInput;

    cout << "Enter the .mdb file name: ";
    getline(cin, fileInput);
    string filename = basePath + fileInput;

    ifstream infile(filename);

    if (infile.fail()) {
        writeToOutputFile("Error opening file: " + filename);
    } else {
        string line;
        while (getline(infile, line)) {
            processLine(line, filename);
        }
        infile.close();
    }

    if (outputFile.is_open()) {
        outputFile.close();  // Close the output file after processing
    }

    return 0;
}
