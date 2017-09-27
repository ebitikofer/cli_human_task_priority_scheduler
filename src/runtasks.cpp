#include "runtasks.h"
#include "helper.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <algorithm>

using namespace std;

RunTasks::RunTasks() {  //set local system time at initial run
	listChanged = false;
	now = time(0);
	ltm = localtime(&now);
	user_dir = getenv("HOME");
	std::cout << "user_dir = " << user_dir << "\n";
	if (loadTasklist("testsave1")) {
		latestMessage = "Loaded saved file!";
	} else {
		latestMessage = "No file to load from (yet)!";
	}
}

void RunTasks::exec() {
	int choice = 0;
	pair<int, int> choice_minmax(0,9);
	do {
		choice = menu(choice_minmax);
		if (latestMessage.size() < 1) {
			latestMessage = "Don't Forget To Save!";
		} else {
			latestMessage.clear();
		}
		switch (choice) {
		case 1:
			createNewTask();
			latestMessage = "New task created!";
			listChanged = true;
		break;
		case 2:
			removeTask();
			listChanged = true;
		break;
		case 3:
			viewAllTasks();
		break;
		case 4:
			sort(allTasks.begin(), allTasks.end());
			latestMessage = "Tasks Sorted!";
			listChanged = true;
		break;
		case 5:
			editTask();
			listChanged = true;
		break;
		case 6:
		break;
		case 7:
		break;
		case 8:
		break;
		case 9:
			if (saveTasklist("testsave1")) {
				latestMessage = "Task state saved!";
				listChanged = false;
			} else if (allTasks.size() < 1) {
				latestMessage = "No tasks on list!";
			} else {
				latestMessage = "Unable to save!";
			}
		break;
		case 0:
			cout << "\n\nGoodBye\n";
		break;
		default:
			cout << "\nInvalid Choice\n";
		break;
		}
	} while (choice != 0);
}


int RunTasks::menu(pair<int, int>& __minmax) {
	clearTerminalScreen();
	viewHighestPriorityTask();
	cout << endl;
	printCurrentTime();
	cout << "\n ->" << latestMessage << "\n\n"
		 << "_____________Main Menu_____________\n"  << left
		 << setw(16) << "1. Add Task"      << " | " << "6. " << endl
		 << setw(16) << "2. Remove Task"   << " | " << "7. " << endl
		 << setw(16) << "3. See Tasks"     << " | " << "8. " << endl
		 << setw(16) << "4. Sort Tasks   " << " | " << "9. Save State" << endl
		 << setw(16) << "5. Edit A Task"   << " | " << "0. Quit Program" << endl;
	return (getNumber("\n   Choice:  ", __minmax.first, __minmax.second));
}

//prompt user via the terminal for a new task entry
void RunTasks::createNewTask() {
	//update time
	now = time(0);
	tm *ltm = localtime(&now);

	clearTerminalScreen();
	string tmpname = "";
	int tmpmonth, tmpday, tmpyear, tmphour, tmpmin;

	do {
		cout << "\nEnter a task name: ";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		getline(cin, tmpname);
		tmpname = reduce(tmpname);  //tighen her up
	} while (tmpname.size() <= 1);  //cant imagine 1 character being enough to desribe a task...

	cout << "\nTask Due Date";

	cout << "\nEnter year: ";
	tmpyear = getNumber(1900 + ltm->tm_year, 9999);

	cout << "\nEnter month: ";
	if (tmpyear == 1900 + ltm->tm_year) tmpmonth = getNumber(1 + ltm->tm_mon,12);  //limit this year to the months still left
	else tmpmonth = getNumber(1, 12);  //otherwise future years any month is fine

	cout << "\nEnter day: ";
	if (tmpyear == 1900 + ltm->tm_year && tmpmonth == 1 + ltm->tm_mon) {
		tmpday = getNumber(ltm->tm_mday, 31);  //limit to the number of days left in the month if task is this month
	} else {
		tmpday = getNumber(1,31);  //otherwise accept any date (always goes to 31, which needs fixed and set based on days in month)
	}
	cout << "\nEnter Hour (24h format): ";
	tmphour = getNumber(0, 23);

	cout << "\nEnter Minute: ";
	tmpmin = getNumber(0, 59);

	Date tmpdate(tmpmonth, tmpday, tmpyear, tmphour, tmpmin);
	Task tmptask(tmpname, tmpdate);
	allTasks.push_back(tmptask);
}

//overloaded version to copy a full vector of already constructed tasks
void RunTasks::createNewTask(vector<Task> &inctasks) {
	allTasks = inctasks;
}


void RunTasks::viewAllTasks() {
	clearTerminalScreen();
	cout << "Here are all the tasks on your list : \n\n";
	if (allTasks.size() < 1) {
		cout << "No Tasks on list.";
		pressEnterToContinue();
		return;
	}
	for (auto & it : allTasks) {
		cout << it.getName() << "\n   Due: "
			 << it.getMonth() << "/" << it.getDay() << "/" << it.getYear()
			 << " @ "
			 << setw(2) << std::setfill('0') << right << it.getHour()
			 << ":"
			 << setw(2) << std::setfill('0') << right << it.getMinute()
			 << endl;
	}
	pressEnterToContinue();
}


void RunTasks::viewHighestPriorityTask() {
	cout << endl;

	/// OPTION 1, NO TASKS ON LIST
	if (allTasks.size() < 1) {
		cout << "No Tasks on list.\n";
		return;


	/// OPTION 2, ONE TASK ON LIST
	} else if (allTasks.size() == 1) {
		cout << "___________Priority Task___________\n\n" << allTasks[0].getName()

		// begin display due time
			 << "\n  Due: " << allTasks[0].getMonth() << "/"
			 << allTasks[0].getDay() << "/" << allTasks[0].getYear()
			 << " @ ";
		string AM_PM = "AM";
		int hour = allTasks[0].getHour();
		if (hour > 12) { hour = hour - 12; AM_PM = "PM"; }
		cout << hour << ":"
			 << setw(2) << std::setfill('0') << right << allTasks[0].getMinute()
			 << AM_PM << endl;
		// end display due time

		return;

	/// OPTION 3, MORE THAN ONE TASK ON LIST
	} else {
		int highestpri = 0;  //vector location of highest priority task found
		int listsize = (int)allTasks.size();  //size of the list
		int i = 0;  //for multiple 'for' loops to follow

		//this loop finds the highest priority task in approx 2n time
		for (i = 0; i < listsize - 1; i++) {
			if (i == 0 && !(allTasks[i].getRawDate() < allTasks[i+1].getRawDate())) {
				highestpri = i+1;
			} else if (!(allTasks[highestpri].getRawDate() < allTasks[i+1].getRawDate())) {
				highestpri = i+1;
			}
		}
		cout << "__________Priority Task(s)__________\n\n1. "
			 << allTasks[highestpri].getName();

		//display due time of first item on the list
		cout << " @";
		string AM_PM = "AM";
		int hour = allTasks[highestpri].getHour();
		if (hour > 12) { hour = hour - 12; AM_PM = "PM"; }
		cout << hour << ":"
			 << setw(2) << std::setfill('0') << right << allTasks[highestpri].getMinute()
			 << AM_PM;
		//end display due time

		//more than one due today
		int count = 2;
		for (i = 0; i < listsize - 1; i++) {
			if (i == highestpri) continue;
			else if (allTasks[i].getYear() == allTasks[highestpri].getYear()) {
				if (allTasks[i].getMonth() == allTasks[highestpri].getMonth()) {
					if (allTasks[i].getDay() == allTasks[highestpri].getDay()) {
						cout << "\n" << count << ". " << allTasks[i].getName();

						//display time of rest of tasks
						cout << " @";
						string AM_PM = "AM";
						int hour = allTasks[i].getHour();
						if (hour > 12) { hour = hour - 12; AM_PM = "PM"; }
						cout << hour << ":"
							 << setw(2) << std::setfill('0') << right << allTasks[i].getMinute()
							 << AM_PM << endl;
						//end display due time

						//display due date for these tasks
						cout << "    Due: " << allTasks[highestpri].getMonth() << "/"
							 << allTasks[highestpri].getDay()
							 << "/" << allTasks[highestpri].getYear();
						count++;
					}
				}
			}
		}
		//end more than one due today

	}
}

void RunTasks::printCurrentTime() {
	//update time
	now = time(0);
	tm *ltm = localtime(&now);
	// print time to screen
	cout << "___________Current Time____________\n\n";
	// print local time
	int hour = ltm->tm_hour;
	string AM_PM = "AM";
	if (hour > 12) { hour = hour - 12; AM_PM = "PM"; }
	cout << std::setfill('0');
	cout << "     ";
	cout << setw(2) << right << hour << ":"
		 << setw(2) << right << ltm->tm_min
		 << AM_PM << "  ";
	//		 << setw(2) << 1 + ltm->tm_sec << " ";  //no seconds
	// print date
	cout << setw(2) << right << 1 + ltm->tm_mon << "/"
		 << setw(2) << right << ltm->tm_mday << "/"
		 << setw(4) << 1900 + ltm->tm_year << endl;
	cout << std::setfill(' ');
}

void RunTasks::removeTask() {
	clearTerminalScreen();
	if (allTasks.size() < 1) {
		cout << "No Tasks on list.";
		pressEnterToContinue();
		return;
	}
	int _count(0);
	int _choice(0);
	cout << "All your Tasks:";
	for (auto & it : allTasks) {
		_count++;
		cout << "\n" << _count << ". "
			 << it.getName() << "\n    Due: "
			 << it.getMonth() << "/" << it.getDay() << "/" << it.getYear()
			 << " @ "
			 << setw(2) << std::setfill('0') << right << it.getHour()
			 << ":"
			 << setw(2) << std::setfill('0') << right << it.getMinute()
			 << endl;
	}
	cout << "\nWhich would you like to remove (by number): ";
	_choice = getNumber(1, _count);
	allTasks.erase(allTasks.begin() - 1 + _choice);
	cout << "\nRemoval complete\n";
	pressEnterToContinue();
}

void RunTasks::editTask() {
	clearTerminalScreen();
	if (allTasks.size() < 1) {
		cout << "No Tasks on list.";
		pressEnterToContinue();
		return;
	}
	int _count(0);
	int _choice(0), _choice2(0);
	cout << "All your Tasks:";
	for (auto & it : allTasks) {
		_count++;
		cout << "\n" << _count << ". "
			 << it.getName() << "\n    Due: "
			 << it.getMonth() << "/" << it.getDay() << "/" << it.getYear()
			 << " @ "
			 << setw(2) << std::setfill('0') << right << it.getHour()
			 << ":"
			 << setw(2) << std::setfill('0') << right << it.getMinute()
			 << endl;
	}
	cout << "\nWhich would you like to edit (by number): ";
	_choice = getNumber(1, _count);
	cout << "\nChange what about it?: \n";
	cout << "1. The Year\n"
		 << "2. The Month\n"
		 << "3. The Day\n"
		 << "4. The Hour\n"
		 << "5. The Minute\n"
		 << "6. The Name\n"
		 << "  Choice: ";
	_choice2 = getNumber(1, 6);
	//update time
	now = time(0);
	tm *ltm = localtime(&now);

	switch (_choice2) {
	case 1: {
		int tmpyear;
		cout << "\nEnter year: ";
		tmpyear = getNumber(1900 + ltm->tm_year, 9999);
		allTasks[_choice - 1].setYear(tmpyear);
	} break;
	case 2: {
		int tmpmonth;
		cout << "\nEnter month: ";
		if (allTasks[_choice - 1].getYear() == 1900 + ltm->tm_year) tmpmonth = getNumber(1 + ltm->tm_mon,12);  //limit this year to the months still left
		else tmpmonth = getNumber(1, 12);  //otherwise future years any month is fine
		allTasks[_choice - 1].setMonth(tmpmonth);
	} break;
	case 3: {
		int tmpday;
		cout << "\nEnter day: ";
		if (allTasks[_choice - 1].getYear() == 1900 + ltm->tm_year && allTasks[_choice - 1].getMonth() == 1 + ltm->tm_mon) {
			tmpday = getNumber(ltm->tm_mday, 31);  //limit to the number of days left in the month if task is this month
		} else {
			tmpday = getNumber(1,31);  //otherwise accept any date (always goes to 31, which needs fixed and set based on days in month)
		}
		allTasks[_choice - 1].setDay(tmpday);
	} break;
	case 4: {
		int tmphour;
		cout << "\nEnter Hour (24h format): ";
		tmphour = getNumber(0, 23);
		allTasks[_choice - 1].setHour(tmphour);
	} break;
	case 5: {
		int tmpmin;
		cout << "\nEnter Minute: ";
		tmpmin = getNumber(0, 59);
		allTasks[_choice - 1].setMinute(tmpmin);
	} break;
	case 6: {
		string tmpname = "";
		do {
			cout << "\nEnter a task name: ";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			getline(cin, tmpname);
			tmpname = reduce(tmpname);  //tighen her up
		} while (tmpname.size() <= 1);  //cant imagine 1 character being enough to desribe a task...
		allTasks[_choice - 1].setName(tmpname);
	} break;
	default:
	break;
	}
	cout << "\nEdit complete\n";
	pressEnterToContinue();
}



//date version - todo: -- use new 'Task' version below
bool operator< (const Date& lhs, const Date& rhs) {
	if (lhs.year <= rhs.year) {
		if (lhs.year < rhs.year) {
			return true;  //year of left hand side is less
		} else {  //years are equal, check month
			if (lhs.month <= rhs.month) {
				if (lhs.month < rhs.month) {
					return true;  //month of left hand side is less
				} else if (lhs.day <= rhs.day) {  //months are equal check day
					if (lhs.day < rhs.day) {
						return true;  //left hand side day is sooner
					} else { //days are equal, check hours
						if (lhs.hour <= rhs.hour) {
							if (lhs.hour < rhs.hour) {
								return true;  //lhs is sooner
							} else {  //hours are the same, check minutes
								if (lhs.minute <= rhs.minute) {
									return true;  //lhs is sooner
								}
							}
						}
					}
				}
			}
		}
	}
	return false;  //rhs is less or lhs == rhs
}

// task version
bool operator< (const Task& T1, const Task& T2) {
	if (T1.getYear() <= T2.getYear()) {
		if (T1.getYear() < T2.getYear()) {
			return true;  //year of left hand side is less
		} else {  //years are equal, check month
			if (T1.getMonth() <= T2.getMonth()) {
				if (T1.getMonth() < T2.getMonth()) {
					return true;  //month of left hand side is less
				} else if (T1.getDay() <= T2.getDay()) {  //months are equal check day
					if (T1.getDay() < T2.getDay()) {
						return true;  //left hand side day is sooner
					} else { //days are equal, check hours
						if (T1.getHour() <= T2.getHour()) {
							if (T1.getHour() < T2.getHour()) {
								return true;  //lhs is sooner
							} else {  //hours are the same, check minutes
								if (T1.getMinute() <= T2.getMinute()) {
									return true;  //lhs is sooner
								}
							}
						}
					}
				}
			}
		}
	}
	return false;  //rhs is less or lhs == rhs
}


//bool operator== (const Date& lhs, const Date& rhs) {
//	if (lhs.year == rhs.year) {
//		if (lhs.month <= rhs.month) {
//			if (lhs.month < rhs.month) {
//				if (lhs.day < rhs.day) {
//					if (lhs.hour <= rhs.hour) {
//						if (lhs.minute <= rhs.minute) {
//							return true;  // dates are equal
//						}
//					}
//				}
//			}
//		}
//	}
//	return false;  // dates are not equal
//}



