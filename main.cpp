#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

// Function prototypes
vector<string> parseCSV(const string& row);
void createFile(const string& filename);
void createTable(const string& line);
void insertIntoTable(const string& line);
void displayTables();
void processLine(const string& line, const string& filename);
void displayDatabasePath(const string& filename);
void writeToOutputFile(const string& message);

vector<string> parseCSV(const string& row) {
    vector<string> columns;
    string current;
    bool insideQuotes = false;

    // Remove the parentheses and process the CSV
    size_t startPos = row.find('(') + 1;
    size_t endPos = row.find(')');
    string cleanedRow = row.substr(startPos, endPos - startPos);  // Remove outer parentheses

    for (char c : cleanedRow) {
        if (c == '\'' && (current.empty() || current.back() != '\\')) {  // Toggle insideQuotes on encountering quotes
            insideQuotes = !insideQuotes;
        }
        else if (c == ',' && !insideQuotes) {  // Split at commas when not inside quotes
            columns.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        columns.push_back(current);  // Push the last column
    }

    // Clean up single quotes around each column value
    for (string& column : columns) {
        if (!column.empty() && column.front() == '\'' && column.back() == '\'') {
            column = column.substr(1, column.length() - 2);  // Remove the surrounding quotes
        }
    }

    return columns;
}


// Store created tables and their data
unordered_set<string> createdTables;
unordered_map<string, vector<string>> tableData;
ofstream outputFile;  // Output file stream

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

    // Remove parentheses and quotes around values
    if (values.front() == '(' && values.back() == ')') {
        values = values.substr(1, values.length() - 2); // Remove outer parentheses
    }

    // Remove single quotes around each value
    size_t posStart = 0;
    while ((posStart = values.find('\'', posStart)) != string::npos) {
        values.erase(posStart, 1);  // Remove single quotes
    }

    tableData[tableName].push_back(values);  // Store the row without parentheses and quotes
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

            // Assuming column names are stored in the first row or manually defined
            writeToOutputFile("customer_id,customer_name,customer_city,customer_state,customer_country,customer_phone,customer_email");

            writeToOutputFile("Data (CSV format):");
            for (const string& row : tableData[tableName]) {
                string cleanedRow = row;

                // Remove parentheses
                cleanedRow.erase(remove(cleanedRow.begin(), cleanedRow.end(), '('), cleanedRow.end());
                cleanedRow.erase(remove(cleanedRow.begin(), cleanedRow.end(), ')'), cleanedRow.end());

                // Remove single quotes from the values
                size_t posStart = 0;
                while ((posStart = cleanedRow.find('\'', posStart)) != string::npos) {
                    cleanedRow.erase(posStart, 1);  // Remove single quotes
                }

                writeToOutputFile(cleanedRow);  // Display data without parentheses and quotes
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
    } else if (line.find("UPDATE") != string::npos) {
        // Parse UPDATE statement
        size_t pos = line.find("UPDATE") + 6;  // Find "UPDATE"
        size_t tableEnd = line.find("SET", pos); // Find the position of "SET"
        string tableName = line.substr(pos, tableEnd - pos); // Extract table name
        tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

        writeToOutputFile("Parsed table name for UPDATE: '" + tableName + "'");

        // Check if the table exists
        if (createdTables.find(tableName) == createdTables.end()) {
            writeToOutputFile("Table does not exist: " + tableName);
            return;
        }

        // Extract the WHERE clause to check for the condition
        size_t wherePos = line.find("WHERE") + 5;
        string whereClause = line.substr(wherePos);
        whereClause.erase(remove_if(whereClause.begin(), whereClause.end(), ::isspace), whereClause.end());

        size_t whereEqualPos = whereClause.find("=");
        string whereColumn = whereClause.substr(0, whereEqualPos);
        string whereValue = whereClause.substr(whereEqualPos + 1);
        whereColumn.erase(remove_if(whereColumn.begin(), whereColumn.end(), ::isspace), whereColumn.end());
        whereValue.erase(remove_if(whereValue.begin(), whereValue.end(), ::isspace), whereValue.end());

        // Check if the table contains data
        if (tableData.find(tableName) != tableData.end()) {
            bool foundRow = false;

            // Loop through the rows and find the matching one
            for (int rowIndex = 0; rowIndex < tableData[tableName].size(); ++rowIndex) {
                string row = tableData[tableName][rowIndex];

                // Parse the row into columns
                vector<string> columns = parseCSV(row);

                bool match = false;

                // Check if this is the column we're interested in
                for (int i = 0; i < columns.size(); ++i) {
                    if (i == 0 && whereColumn == "customer_id") {  // Assuming customer_id is the first column
                        if (columns[i] == whereValue) {
                            match = true;
                            break;
                        }
                    }
                }

                if (match) {
                    // Update the row in CSV format
                    string updatedRow;
                    size_t valueStart = 0;
                    size_t valueEnd = row.find(',');

                    for (int i = 0; i < columns.size(); ++i) {
                        if (i == 6) {  // Assuming customer_email is at index 6
                            updatedRow += "email333";  // Update the email
                        } else {
                            updatedRow += columns[i];  // Keep other columns unchanged
                        }
                        if (i < columns.size() - 1) {
                            updatedRow += ",";  // Add a comma between columns
                        }
                    }

                    writeToOutputFile("Updated row: " + updatedRow);
                    tableData[tableName][rowIndex] = updatedRow;  // Save the updated row
                    break;  // Exit loop after finding and updating the matching row
                }
            }
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
