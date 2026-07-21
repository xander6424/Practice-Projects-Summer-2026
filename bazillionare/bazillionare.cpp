#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <cctype>
#include <chrono>

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
        https://the-trivia-api.com/

    This program plays a trivia game that tracks and carries over user data in between separate runs
    of the program. Players can select different modes, look at leaderboard stats, and compete for
    high scores by utilizing the trivia API.

*/


// Info for each answer option in the multiple choice
struct Multiple_Choice {
    string text;
    bool correct;
};


struct Question_Info {
    string question = ""; // accessed by data[i]["question"]["text"]
    std::vector<Multiple_Choice> answer_list = {};
    string id = "";

    bool is_valid = true;
};


struct User_Info {
    string username = "";
    int high_score_normal = 0;
    int high_score_hard = 0;
    int high_score_endless = 0;
    double time_normal = 0.0;
    double time_hard = 0.0;
};


struct User_Data {
    std::vector<User_Info> user_list;
    User_Info current_user;
};


class Trivia_API {
    public:
        std::vector<Question_Info> generate_questions(int difficulty) {
            cout << "GENERATING QUESTIONS...\n";

            // Create API client through SSL
            httplib::SSLClient cli("the-trivia-api.com");
            cout << "Client created...\n";

            // Prevents HTTPS redirects
            cli.enable_server_certificate_verification(false);
            cli.set_follow_location(true);

            // Time to wait for connection to establish and for recieving data
            cli.set_connection_timeout(5);
            cli.set_read_timeout(5);

            // Sends request for questions depending on difficulty
            cout << "Sending request...\n";
            std::vector<string> requests;
            switch (difficulty) {
                // Easy (5 easy, 5 medium, and 5 hard questions)
                case 1:
                    requests.push_back("/v2/questions?limit=5&difficulties=easy&types=text_choice");
                    requests.push_back("/v2/questions?limit=5&difficulties=medium&types=text_choice");
                    requests.push_back("/v2/questions?limit=5&difficulties=hard&types=text_choice");
                    break;
                // Hard (15 hard questions)
                case 2:
                    requests.push_back("/v2/questions?limit=15&difficulties=hard&types=text_choice");
                    break;
                // Endless (15 random questions that generates again when exceeded)
                case 3:
                    requests.push_back("/v2/questions?limit=15&types=text_choice");
                    break;
            };

            std::vector<Question_Info> questions;

            for (int i = 0; i < requests.size(); i++) {

                auto result = cli.Get(requests[i]);

                // Only checks for a successful connection ONE TIME
                if (i == 0) {
                    cout << "Request completed...\n";

                    if (result && result->status == 200) {
                        cout << "SUCCESS!\n";
                    } else {
                        cout << "FAILED.\n";

                        if (result) {
                            cout << "HTTP Status: " << result->status << endl;

                            Question_Info message;
                            message.is_valid = false;
                            return { message };

                        } else {
                            auto error = result.error();

                            cout << "Error Code: " << static_cast<int>(error) << endl;

                            Question_Info message;
                            message.is_valid = false;
                            return { message };
                        }
                    }
                }

                // Makes json data readable in C++
                json data = json::parse(result->body);

                // Loops through each question and adds its data
                for (int i = 0; i < data.size(); i++) {
                    Question_Info question_instance;
                    question_instance.question = data[i]["question"]["text"];
                    question_instance.id = data[i]["id"];

                    Multiple_Choice correct;
                    correct.text = data[i]["correctAnswer"];
                    correct.correct = true;
                    question_instance.answer_list.push_back(correct);

                    // Add incorrect answers to the list
                    for (int j = 0; j < data[i]["incorrectAnswers"].size(); j++) {
                        Multiple_Choice incorrect;
                        incorrect.text = data[i]["incorrectAnswers"][j];
                        incorrect.correct = false;
                        question_instance.answer_list.push_back(incorrect);
                    }

                    questions.push_back(question_instance);
                }
            }
            
            return questions;
        }
};


// Function declarations
void print_menu(int menu);
void print_leaderboard(int difficulty, User_Data user_data);
User_Data get_user_data();
User_Data add_user(User_Data user_data);
User_Data update_user(User_Data user_data, int current_score, int minutes, int seconds, int difficulty);
User_Data bazillionare(int difficulty, User_Data user_data);
Question_Info fifty_fifty(Question_Info q, std::mt19937 g);
void phone_friend(Question_Info q, std::mt19937 g);


int main() {
    int choice;
    int difficulty;
    User_Data user_data = get_user_data();
    user_data = add_user(user_data);

    do {
        print_menu(1);
        cin >> choice;

        // Begin game
        if (choice == 1) {
            // Select difficulty (normal/hard/endless)
            do {
                print_menu(2);
                cin >> choice;

                switch(choice) {
                    case 1:
                        cout << "Normal mode selected.\n";
                        difficulty = 1;
                        break;
                    case 2:
                        cout << "Hard mode selected.\n";
                        difficulty = 2;
                        break;
                    case 3:
                        cout << "Endless mode selected.\n";
                        difficulty = 3;
                        break;
                    default:
                        cout << "Invalid option selected. You are NOT a bazillionare.\n";
                        break;
                }
            } while (choice != 1  && choice != 2 && choice != 3);

            // PLAY THE GAME AND RETURN USER RESULTS
            user_data = bazillionare(difficulty, user_data);

            // Resets choice after completing the game
            choice = 0;

        // Print leaderboards
        } else if (choice == 2) {
            do {
                print_menu(2);
                cin >> choice;
            
                switch(choice) {
                    case 1:
                        cout << "Normal mode selected.\n";
                        difficulty = 1;
                        break;
                    case 2:
                        cout << "Hard mode selected.\n";
                        difficulty = 2;
                        break;
                    case 3:
                        cout << "Endless mode selected.\n";
                        difficulty = 3;
                        break;
                    default:
                        cout << "Invalid option selected. You are NOT a bazillionare.\n";
                        break;
                }
            } while (choice != 1  && choice != 2 && choice != 3);

            print_leaderboard(choice, user_data);

            // Resets choice after completing the game
            choice = 0;
        // Exit program
        } else if (choice == 3) {
            cout << "Exiting program. You are NOT a bazillionare.\n";
        } else {
            cout << "Invalid option selected. You are NOT a bazillionare.\n";
        }
    } while (choice != 3);

    return 0;
}


User_Data bazillionare(int difficulty, User_Data user_data) {
    User_Info player = user_data.current_user;
    Trivia_API api_instance;
    char options[4] = {'A', 'B', 'C', 'D'};
    int current_score = 0;
    int endless_temp_score = 0;
    int user_answer;
    bool playing = true;
    bool fifty = true;
    bool phone = true;

    // Generate questions from API based on difficulty
    std::vector<Question_Info> questions = api_instance.generate_questions(difficulty);
    if (questions[0].is_valid == false) {
        cout << "Unable to generate questions. Exiting game instance...\n";
        return user_data;
    }

    // Print game rules to buffer
    print_menu(3);

    // Prepare random seed and start timer
    std::random_device rd;
    std::mt19937 g(rd());
    auto start = std::chrono::steady_clock::now();

    // Start the actual game
    while (current_score < 15 && playing) {
        Question_Info q = questions[current_score];
        std::shuffle(q.answer_list.begin(), q.answer_list.end(), g);

        do {
            // Print question
            cout << endl << q.question << endl;

            // Print question options with text
            for (int i = 0; i < q.answer_list.size(); i++) {
                cout << options[i] << ". " << q.answer_list[i].text << endl;
            }

            char user_input;
            cout << "Enter answer: ";
            cin >> user_input;
            cout << endl;
            user_input = (char)std::toupper(user_input);

            switch (user_input) {
                case 'A':
                    user_answer = 0;
                    break;
                case 'B':
                    user_answer = 1;
                    break;
                case 'C':
                    user_answer = 2;
                    break;
                case 'D':
                    user_answer = 3;
                    break;
                default:
                    if (fifty || phone) {
                        // Prompts for help choice
                        cout << "(1) Cut answers in half\n";
                        cout << "(2) Phone a friend\n";
                        cout << "Enter answer: ";
                        cin >> user_answer;

                        switch (user_answer) {
                            // Cut answers in half
                            case 1:
                                if (fifty) {
                                    q = fifty_fifty(q, g);

                                    fifty = false;
                                } else {
                                    cout << "This help option has already been used.\n";
                                }
                                
                                break;
                            // Phone a friend
                            case 2:
                                if (phone) {
                                    phone_friend(q, g);

                                    phone = false;
                                } else {
                                    cout << "This help option has already been used.\n";
                                }
                                
                                break;
                            default:
                                cout << "Invalid option selected.\n";
                                break;
                        };
                    } else {
                        cout << "All help has been used.\n";
                    }

                    // Ask question again after help/invalid answer
                    user_answer = -1;
                    break;
            };

            // Checks if C or D were picked after a fifty fifty
            if (user_answer >= (int)q.answer_list.size()) {
                cout << "Invalid option selected.\n";
                user_answer = -1;
            }
        } while (user_answer == -1);

        if (q.answer_list[user_answer].correct) {
            current_score++;
            cout << "CORRECT!!! Score: " << (current_score + endless_temp_score) << endl;

            // In endless mode, keep asking questions if all 15 have been used
            if (current_score == 15 && difficulty == 3) {
                endless_temp_score += current_score;
                current_score = 0;
                questions = api_instance.generate_questions(difficulty);
            }
        } else {
            cout << "INCORRECT!!!";

            // Print the correct answer
            for (int i = 0; i < q.answer_list.size(); i++) {
                if (q.answer_list[i].correct) {
                    cout << " Correct Answer: " << options[i] << ". " << q.answer_list[i].text << endl;
                }
            }

            playing = false;
        }
    }

    // Finalize time across all modes
    auto end = std::chrono::steady_clock::now();
    auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        end - start
    ).count();
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;

    // Add total endless score together
    if (difficulty == 3) {
        current_score += endless_temp_score;
    }

    // Print results and message
    if (playing) {
        cout << "\n******   YOU WON 'WHO DOESN'T WANT TO BE A BAZILLIONARE??!!'   ******\n";
        print_menu(4);
    }

    cout << "FINAL SCORE: " << current_score << endl;

    if (difficulty != 3) {
        cout << "FINAL TIME: " << minutes << " minutes & " << seconds << " seconds\n\n";
    }

    print_menu(5);
    user_data = update_user(user_data, current_score, minutes, seconds, difficulty);
    return user_data;
}


User_Data get_user_data() {
    User_Data user_data;
    std::vector<User_Info> user_list;
    std::ifstream fin("user-data.txt");

    // Checks for invalid file
    if (!fin) {
        cout << "No user data detected. Creating new file...\n";
        std::ofstream created_file("user-data.txt");
        created_file.close();
    } else {
        cout << "Gathering user data...\n";
    }

    fin.close();

    // Begins reading existing file data into vector
    fin.open("user-data.txt");
    User_Info temp;

    // Reads each set of data from file and appends to the user list
    while (getline(fin, temp.username)) {
        fin >> temp.high_score_normal;
        fin >> temp.high_score_hard;
        fin >> temp.high_score_endless;
        fin >> temp.time_normal;
        fin >> temp.time_hard;
        fin.ignore();

        user_list.push_back(temp);
    }

    user_data.user_list = user_list;

    fin.close();

    return user_data;
}


User_Data add_user(User_Data user_data) {
    // Gets username to create or find current user
    string username;
    cout << "Please enter your username: ";
    getline(cin, username);

    // Searches list of users for an existing match
    for (int i = 0; i < user_data.user_list.size(); i++) {
        // Select user input as the current user
        if (username == user_data.user_list[i].username) {
            cout << "Existing user " << username << " found.\n";
            user_data.current_user = user_data.user_list[i];

            return user_data;
        }
    }

    cout << "No match found for " << username << ". Creating new user...\n";

    // Add new user to the list
    User_Info new_user;
    new_user.username = username;
    user_data.current_user = new_user;
    user_data.user_list.push_back(new_user);
    
    // Writes new user to the data file
    std::ofstream fout("user-data.txt", std::ios::app);

    fout << new_user.username << endl;
    fout << new_user.high_score_normal << endl;
    fout << new_user.high_score_hard << endl;
    fout << new_user.high_score_endless << endl;
    fout << new_user.time_normal << endl;
    fout << new_user.time_hard << endl;

    fout.close();

    return user_data;
}


User_Data update_user(User_Data user_data, int current_score, int minutes, int seconds, int difficulty) {
    std::ifstream fin("user-data.txt");

    // Checks for invalid file
    if (!fin) {
        cout << "No user data detected. File may have been deleted.\n";
        return user_data;
    } else {
        cout << "Updating user data...\n";
    }

    int total_seconds = seconds + (minutes * 60);
    
    // Assign user results to correct variables in user data
    switch (difficulty) {
        // Normal
        case 1:
            if (current_score > user_data.current_user.high_score_normal) {
                user_data.current_user.high_score_normal = current_score;

                if (user_data.current_user.time_normal == 0 || total_seconds < user_data.current_user.time_normal) {
                    user_data.current_user.time_normal = total_seconds;
                }
            }
            
            break;
        // Hard
        case 2:
            if (current_score > user_data.current_user.high_score_hard) {
                user_data.current_user.high_score_hard = current_score;

                if (user_data.current_user.time_hard == 0 || total_seconds < user_data.current_user.time_hard) {
                    user_data.current_user.time_hard = total_seconds;
                }
            }

            break;
        // Endless
        case 3:
            if (current_score > user_data.current_user.high_score_endless) {
                user_data.current_user.high_score_endless = current_score;
            }
            
            break;
    }

    string current_username = user_data.current_user.username;
    std::vector<std::string> file_lines;
    string temp;

    // Read each line into a vector
    while (getline(fin, temp)) {
        file_lines.push_back(temp);
    }

    fin.close();

    // Replace correct lines with new user data
    for (int i = 0; i < file_lines.size(); i++) {
        if (file_lines[i] == current_username) {
            file_lines[i+1] = std::to_string(user_data.current_user.high_score_normal);
            file_lines[i+2] = std::to_string(user_data.current_user.high_score_hard);
            file_lines[i+3] = std::to_string(user_data.current_user.high_score_endless);
            file_lines[i+4] = std::to_string(user_data.current_user.time_normal);
            file_lines[i+5] = std::to_string(user_data.current_user.time_hard);

            break;
        }
    }

    // Write new lines back to file
    std::ofstream fout("user-data.txt");

    for (string line : file_lines) {
        fout << line << endl;
    }

    fout.close();

    // Add new user data into the user_data variable
    user_data = get_user_data();

    // Update the current user back into the whole list
    for (int i = 0; i < user_data.user_list.size(); i++) {
        if (user_data.user_list[i].username == current_username) {
            user_data.current_user = user_data.user_list[i];
            break;
        }
    }

    cout << "User data updated.\n";
    return user_data;
}


void print_menu(int menu) {
    // Input variable for buffering the program
    int accept = 0;

    // Prints main menu
    if (menu == 1) {
        cout << "\nO--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O\n";
        cout << "|                                                                       |\n";
        cout << "O               WHO DOESN'T WANT TO BE A BAZILLIONARE??!!               O\n";
        cout << "|                                                                       |\n";
        cout << "O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O--O\n\n";
        
        cout << "(1) Play 'Who Doesn't Want to be a Bazillionare??!!'\n";
        cout << "(2) Show 'Who Doesn't Want to be a Bazillionare??!!' leaderboard\n";
        cout << "(3) Quit 'Who Doesn't Want to be a Bazillionare??!!'\n";
        
        cout << "\nSelect option: ";
    // Prints difficulty submenu
    } else if (menu == 2) {
        cout << "\n(1) Normal\n";
        cout << "(2) Hard\n";
        cout << "(3) Endless\n";

        cout << "\nSelect difficulty: ";
    // Prints game rules
    } else if (menu == 3) {
        cout << "\n******   RULES OF 'WHO DOESN'T WANT TO BE A BAZILLIONARE??!!'   ******\n";

        cout << "1. Answer questions as they appear.\n";
        cout << "2. One incorrect answer will end the game.\n";
        cout << "3. Enter anything besides the four given letter options for hints.\n";
        cout << "4. Answer 15 questions to win! (besides endless mode)\n";
        cout << "5. Try to win as quickly as possible.\n\n";
    
        while (accept != 1) {
            cout << "Press 1 to start 'Who Doesn't Want to be a Bazillionare??!!': ";
            cin >> accept;
        }
    // Prints win event
    } else if (menu == 4) {
        while (accept != 1) {
            cout << "Press 1 to accept your prize of one bazillion dollars: ";
            cin >> accept;
        }

        cout << "$1,";
        for (int i = 0; i < 10000; i++) {
            cout << "000,";
        }
        cout << "000\n";
    // Prints basic buffer
    } else {
        while (accept != 1) {
            cout << "Press 1 to continue: ";
            cin >> accept;
        }
    }
}


void print_leaderboard(int difficulty, User_Data user_data) {
    int users = user_data.user_list.size();
    User_Info temp;
    int count = 1;
    int minutes;
    int seconds;

    // Swaps users' scores and times of each difficulty only if they're improved
    switch (difficulty) {
        // Normal leaderboard
        case 1:
            for (int i = 0; i < users - 1; i++) {
                for (int j = 0; j < users - i - 1; j++) {
                    if (user_data.user_list[j].high_score_normal > user_data.user_list[j+1].high_score_normal) {
                        temp = user_data.user_list[j];
                        user_data.user_list[j] = user_data.user_list[j+1];
                        user_data.user_list[j+1] = temp;
                    } else {
                        if (user_data.user_list[j].high_score_normal == user_data.user_list[j+1].high_score_normal) {
                            if (user_data.user_list[j].time_normal < user_data.user_list[j+1].time_normal) {
                                temp = user_data.user_list[j];
                                user_data.user_list[j] = user_data.user_list[j+1];
                                user_data.user_list[j+1] = temp;
                            }
                        }
                    }
                }
            }

            cout << "\n************ NORMAL MODE LEADERBOARD ************\n";
            for (int i = users - 1; i >= 0; i--) {
                // Only print the user if they have played that mode
                if (user_data.user_list[i].time_normal != 0) {
                    minutes = static_cast<int>(user_data.user_list[i].time_normal) / 60;
                    seconds = static_cast<int>(user_data.user_list[i].time_normal) % 60;

                    cout << count << ". " << user_data.user_list[i].username
                        << " | Score: " << user_data.user_list[i].high_score_normal
                        << " | Time: " << minutes << " minutes & " << seconds << " seconds\n";
                    
                    count++;
                }
            }

            if (count == 1) {
                cout << "          No user data for normal mode.\n";
            }

            break;
        // Hard leaderboard
        case 2:
            for (int i = 0; i < users - 1; i++) {
                for (int j = 0; j < users - i - 1; j++) {
                    if (user_data.user_list[j].high_score_hard > user_data.user_list[j+1].high_score_hard) {
                        temp = user_data.user_list[j];
                        user_data.user_list[j] = user_data.user_list[j+1];
                        user_data.user_list[j+1] = temp;
                    } else {
                        if (user_data.user_list[j].high_score_hard == user_data.user_list[j+1].high_score_hard) {
                            if (user_data.user_list[j].time_hard < user_data.user_list[j+1].time_hard) {
                                temp = user_data.user_list[j];
                                user_data.user_list[j] = user_data.user_list[j+1];
                                user_data.user_list[j+1] = temp;
                            }
                        }
                    }
                }
            }

            cout << "\n************ HARD MODE LEADERBOARD ************\n";
            for (int i = users - 1; i >= 0; i--) {
                // Only print the user if they have played that mode
                if (user_data.user_list[i].time_hard != 0) {
                    minutes = static_cast<int>(user_data.user_list[i].time_hard) / 60;
                    seconds = static_cast<int>(user_data.user_list[i].time_hard) % 60;

                    cout << count << ". " << user_data.user_list[i].username
                        << " | Score: " << user_data.user_list[i].high_score_hard
                        << " | Time: " << minutes << " minutes & " << seconds << " seconds\n";
                    
                    count++;
                }
            }

            if (count == 1) {
                cout << "          No user data for hard mode.\n";
            }

            break;
        // Endless leaderboard
        case 3:
            for (int i = 0; i < users - 1; i++) {
                for (int j = 0; j < users - i - 1; j++) {
                    if (user_data.user_list[j].high_score_endless > user_data.user_list[j+1].high_score_endless) {
                        temp = user_data.user_list[j];
                        user_data.user_list[j] = user_data.user_list[j+1];
                        user_data.user_list[j+1] = temp;
                    }
                }
            }

            cout << "\n************ ENDLESS MODE LEADERBOARD ************\n";
            for (int i = users - 1; i >= 0; i--) {
                // Only print the user if they have played that mode
                if (user_data.user_list[i].high_score_endless != 0) {
                    cout << count << ". " << user_data.user_list[i].username
                        << " | Score: " << user_data.user_list[i].high_score_endless << endl;
                    
                    count++;
                }
            }

            if (count == 1) {
                cout << "          No user data for endless mode.\n";
            }

            break;
    }

    cout << endl;
    print_menu(5);
}


Question_Info fifty_fifty(Question_Info q, std::mt19937 g) {
    string correct_answer;
    int removed = 2;

    // Finds what the correct answer is
    for (int i = 0; i < q.answer_list.size(); i++) {
        if (q.answer_list[i].correct) {
            correct_answer = q.answer_list[i].text;
        }
    }

    // Removes 2 random incorrect answers
    while (removed > 0) {
        std::uniform_int_distribution<> dist(0, q.answer_list.size() - 1);
        int index = dist(g);

        if (q.answer_list[index].text != correct_answer) {
            q.answer_list.erase(q.answer_list.begin() + index);
            removed--;
        }
    }
    
    return q;
}


void phone_friend(Question_Info q, std::mt19937 g) {
    std::uniform_int_distribution<> dist1(0, 4);
    std::uniform_int_distribution<> dist2(1, 5);
    std::uniform_int_distribution<> dist3(0, 2);
    int rand_message = dist1(g);
    int rand_answer = dist2(g);
    int rand_index = dist3(g);
    
    string given_answer;
    string correct_answer;
    std::vector<string> incorrect_answers;
    string messages[5] = {
        "\"I'm pretty sure the answer is ",
        "\"I'd personally say that it's ",
        "\"If there was a gun to my head, I'd say ",
        "\"If it were me, I'd say it's ",
        "\"I think the answer is "
    };

    // Separates correct and incorrect answers
    for (int i = 0; i < q.answer_list.size(); i++) {
        if (q.answer_list[i].correct) {
            correct_answer = q.answer_list[i].text;
        } else {
            incorrect_answers.push_back(q.answer_list[i].text);
        }
    }

    // Checks if fifty fifty has been used to prevent access outside current index
    if (!incorrect_answers.empty()) {
        std::uniform_int_distribution<> dist3(0, incorrect_answers.size() - 1);
        rand_index = dist3(g);
    }

    // Gives the correct answer (80% chance)
    if (rand_answer != 1) {
        given_answer = correct_answer;
    // Gives the incorrect answer (20% chance)
    } else {
        given_answer = incorrect_answers[rand_index];
    }

    cout << endl << messages[rand_message] << "'" << given_answer << "'.\"\n";
}