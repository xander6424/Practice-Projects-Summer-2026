#include <iostream>
#include <fstream>
#include <string>
#include <set>

// API Libraries
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"
using json = nlohmann::json;

using std::cout;
using std::cin;
using std::endl;
using std::string;


/*

    The following program utilizes the following libraries/APIs on GitHub:
        https://github.com/yhirose/cpp-httplib
        https://github.com/nlohmann/json
        https://github.com/lukePeavey/quotable

    This program allows the user to generate or find famous quotes from the Quotable API by
    lukePeavey on GitHub. These quotes can then be stored to an alphabetical, editable list 
    of favorites which can be exported to an external .txt file.

*/


struct Quote_Info {
    string text = "";
    string author = "";
    string id = "";

    // Override set comparison to sort alphabetically
    bool operator<(const Quote_Info& other) const {
        return text < other.text;
    }
};


class Quote_API {
    public:
        Quote_Info generate_quote(string unique_id) {
            cout << "STARTING QUOTE GENERATION\n";

            // Create API client through SSL
            httplib::SSLClient cli("api.quotable.io");
            Quote_Info current_quote;
            cout << "Client created...\n";

            // Prevents HTTPS redirects
            cli.enable_server_certificate_verification(false);
            cli.set_follow_location(true);

            // Time to wait for connection to establish and for recieving data
            cli.set_connection_timeout(5);
            cli.set_read_timeout(5);

            // Sets the header for each API call
            cli.set_default_headers({{"User-Agent", "CppApp"}});

            // Sends request for either random quote or quote by ID
            cout << "Sending request...\n";
            auto result = (unique_id.empty()) ? cli.Get("/random") : cli.Get(unique_id);
            cout << "Request completed...\n";

            // Checks for valid result
            if (result && result->status == 200) {
                // Makes json data understandable in C++
                json data = json::parse(result->body);

                cout << "SUCCESS!\n";

                current_quote.text = data["content"];
                current_quote.author = data["author"];
                current_quote.id = data["_id"];

                return current_quote;
            }
            else {
                cout << "FAILED.\n";

                // Return HTTP Status if result connected but failed
                if (result) {
                    cout << "HTTP Status: ";
                    cout << result->status << endl;

                    return current_quote;
                }
                // Return error code if result failed to connect
                else {
                    auto error = result.error();

                    cout << "Error Code: ";
                    cout << static_cast<int>(error) << endl;

                    return current_quote;
                }
            }
        }
};


void print_main_menu();
void print_sub_menu();
void print_favorite_quotes(std::set<Quote_Info> favorite_quotes);
void print_favorite_quotes_to_file(std::set<Quote_Info> favorite_quotes);


int main() {
    cout << "*********************************************\n";
    cout << "|                                           |\n";
    cout << "|  'QUOTABLE' API QUOTE GENERATION PROGRAM  |\n";
    cout << "|                                           |\n";
    cout << "*********************************************\n";

    int choice;
    string unique_id = "";
    Quote_API api_instance;
    Quote_Info current_quote;
    Quote_Info previous_quote;
    std::set<Quote_Info> favorite_quotes;

    do {
        print_main_menu();
        cin >> choice;

        if (choice == 1 || choice == 2) {
            // Generate random quote
            if (choice == 1) {
                unique_id = "";
                current_quote = api_instance.generate_quote(unique_id);
            // Generate quote by ID
            } else {
                cout << "Enter quote ID to search for: ";
                cin >> unique_id;

                // Create new Get command
                unique_id = "/quotes/" + unique_id;
                current_quote = api_instance.generate_quote(unique_id);
            }
            
            if (current_quote.id != "") {
                cout << "\nGENERATED QUOTE:\n";
                cout << "Text: \"" << current_quote.text << "\"\n";
                cout << "Author: - " << current_quote.author << "\n";
                cout << "ID: " << current_quote.id << "\n";
            } else {
                cout << "Unable to generate quote.\n";
            }
        // Favorite currently generated quote
        } else if (choice == 3) {
            // Prevents saving the same quote twice (set also covers this)
            if (current_quote.id != previous_quote.id) {
                previous_quote = current_quote;
                favorite_quotes.insert(current_quote);
                cout << "Added current quote to favorites.\n";
            } else if (current_quote.id == "") {
                cout << "No quote has been generated.\n";
            } else {
                cout << "Unable to favorite quote.\n";
            }
        // Display favoried quote list
        } else if (choice == 4) {
            if (favorite_quotes.size() > 0) {
                do {
                    print_favorite_quotes(favorite_quotes);
                    print_sub_menu();
                    cin >> choice;

                    // Removes quote by ID
                    if (choice == 1) {
                        cout << "Enter quote ID to remove: ";
                        cin >> unique_id;

                        bool removed = false;

                        // Search for matching ID in favorite quotes to remove from
                        for (Quote_Info quote : favorite_quotes) {
                            if (quote.id == unique_id) {
                                favorite_quotes.erase(quote);
                                cout << "Successfully removed quote from favorites.\n";
                                removed = true;

                                if (favorite_quotes.size() == 0) {
                                    cout << "There are no more quotes in favorites.\n";
                                }
                            }
                        }

                        if (!removed) {
                            cout << "Unable to remove quote from favorites.\n";
                        }
                    // Print favorites list to file
                    } else if (choice == 2) {
                        print_favorite_quotes_to_file(favorite_quotes);
                    // Returns to main menu
                    } else if (choice == 3) {
                        cout << "Returning to main menu...\n";
                    } else {
                        cout << "Invalid option selected.\n";
                    }
                } while (choice != 3 && favorite_quotes.size() > 0);
            }
            else {
                cout << "No favorited quotes.\n";
            }
        // Closes program
        } else if (choice == 5) {
            cout << "Exiting program...\n";
        } else {
            cout << "Invalid option selected.\n";
        }
    } while (choice != 5);

    return 0;
}


void print_main_menu() {
    cout << "\n(1) Generate random quote\n";
    cout << "(2) Generate quote by ID\n";
    cout << "(3) Favorite current quote\n";
    cout << "(4) Display favorite quotes\n";
    cout << "(5) Exit program\n";

    cout << "\nSelect choice: ";
}


void print_sub_menu() {
    cout << "\n(1) Remove quote by ID\n";
    cout << "(2) Print favorite quotes to file\n";
    cout << "(3) Return to main menu\n";

    cout << "\nSelect choice: ";
}


void print_favorite_quotes(std::set<Quote_Info> favorite_quotes) {
    cout << "\n********************************\n";
    cout << "|                              |\n";
    cout << "|       FAVORITED QUOTES       |\n";
    cout << "|                              |\n";
    cout << "********************************\n";
    
    int count = 1;
    
    for (Quote_Info quote : favorite_quotes) {
        cout << count << ". \"" << quote.text << "\"\n";
        cout << "   - " << quote.author << " (" << quote.id << ")\n\n";

        count++;
    }
}


void print_favorite_quotes_to_file(std::set<Quote_Info> favorite_quotes) {
    // Replaces all cout with File
    std::ofstream File("favorite-quotes-list.txt");

    File << "********************************\n";
    File << "|                              |\n";
    File << "|       FAVORITED QUOTES       |\n";
    File << "|                              |\n";
    File << "********************************\n";
    
    int count = 1;
    
    for (Quote_Info quote : favorite_quotes) {
        File << count << ". \"" << quote.text << "\"\n";
        File << "   - " << quote.author << " (" << quote.id << ")\n\n";

        count++;
    }

    cout << "Successfully created file.\n";
    File.close();
}