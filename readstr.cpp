#include<iostream>
#include <fstream>
#include<string>
using namespace std;
class Student
{
public:
	// data members
	char* id, *address, *first_enroll, *credit_hour;
	// method
	Student() {
		id = new char[20];
		address = new char[20];
		first_enroll = new char[20];
		credit_hour = new char[20];
	}
	// default constructor
	Student(char* _id, char* _address, char* _first_enroll, char* _credit_hour) :
		id(_id), address(_address), first_enroll(_first_enroll), credit_hour(_credit_hour) {}
	Student& changeValue(char* _id, char* _address, char* _first_enroll, char* _credit_hour) {
		id = _id;
		address = _address;
		first_enroll = _first_enroll;
		credit_hour = _credit_hour;
	}
	Student operator=(Student from) {
		this->id = from.id;
		this->address = from.address;
		this->first_enroll = from.first_enroll;
		this->credit_hour = from.credit_hour;

		return *this;
	}
	friend istream & operator >> (istream & is, Student & s);
	friend ostream & operator << (ostream & os, Student & s);
};


istream & operator >> (istream & is, Student & s)
{ // read fields from 
	
	cout << "(Student)Enter<y> continue, or <ctr+z> to end: " << flush;
	is.getline(s.id, 30);
	if (strlen(s.id) == 0) return is;
	cout << "Enter id: " << flush; is.getline(s.id, 30);
	cout << "Enter address: " << flush; is.getline(s.address, 30);
	cout << "Enter first enroll: " << flush; is.getline(s.first_enroll, 30);
	cout << "Enter credit hour: " << flush; is.getline(s.credit_hour, 30);
	return is;
}
ostream & operator << (ostream & os, Student & s)
{ // insert fields into file
	os << s.id << s.address << s.first_enroll
		<< s.credit_hour;
	return os;
}


class CourseRegistration
{
public:
	// data members
	char* course_id, *student_id, *course_grade, *credit_hour;
	// method
	CourseRegistration() {
		course_id = new char[20];
		student_id = new char[20];
		course_grade = new char[20];
		credit_hour = new char[20];
	}
	// default constructor
	CourseRegistration(char* _course_id, char* _student_id, char* _course_grade, char* _credit_hour) :
		course_id(_course_id), student_id(_student_id), course_grade(_course_grade), credit_hour(_credit_hour) {}
	CourseRegistration& changeValue(char* _course_id, char* _student_id, char* _course_grade, char* _credit_hour) {
		course_id = _course_id;
		student_id= _student_id;
		course_grade = _course_grade;
		credit_hour = _credit_hour;
	}
	CourseRegistration operator=(CourseRegistration from) {
		this->course_id = from.course_id;
		this->student_id = from.student_id;
		this->course_grade = from.course_grade;
		this->credit_hour = from.credit_hour;

		return *this;
	}
	friend istream & operator >> (istream & is, CourseRegistration & s);
	friend ostream & operator << (ostream & os, CourseRegistration & s);
};


istream & operator >> (istream & is, CourseRegistration & s)
{ // read fields from 
	cout << "(CourseRegistration)Enter<y> continue, or <ctr+z> to end: " << flush;
	is.getline(s.course_id, 30);
	if (strlen(s.course_id) == 0) return is;
	cout << "Enter course id: " << flush; is.getline(s.course_id, 30);
	cout << "Enter student id: " << flush; is.getline(s.student_id, 30);
	cout << "Enter course grade: " << flush; is.getline(s.course_grade, 30);
	cout << "Enter credit hour: " << flush; is.getline(s.credit_hour, 30);
	return is;
}
ostream & operator << (ostream & os, CourseRegistration & s)
{ // insert fields into file
	os << s.course_id << s.student_id << s.course_grade
		<< s.credit_hour;
	return os;
}
class Course
{
public:
	// data members
	char* course_id, *course_name, *course_grade,
		*course_hour,*teacher,*classroom;
	// method
	Course() {
		course_id = new char[20];
		course_name = new char[20];
		course_grade = new char[20];
		course_hour = new char[20];
		teacher = new char[20];
		classroom = new char[20];
	}
	// default constructor
	Course(char* _course_id, char* _course_name, char* _course_grade, char* _course_hour
	,char* _teacher,char* _classroom) :
		course_id(_course_id), course_name(_course_name), course_grade(_course_grade), course_hour(_course_hour)
	,teacher(_teacher), classroom(_classroom){}
	Course& changeValue(char* _course_id, char* _course_name, char* _course_grade, char* _course_hour
		, char* _teacher, char* _classroom) {
		course_id = _course_id;
		course_name = _course_name;
		course_grade = _course_grade;
		course_hour = _course_hour;
		teacher = _teacher;
		classroom = _classroom;
	}
	Course operator=(Course from) {
		this->course_id = from.course_id;
		this->course_name = from.course_name;
		this->course_grade = from.course_grade;
		this->course_hour = from.course_hour;
		this->teacher = from.teacher;
		this->classroom = from.classroom;

		return *this;
	}
	
};

int main() {
	char filename[20];
	Student p;
	CourseRegistration C;
	cout << "Enter the file name:" << flush;
	cin.getline(filename, 19);
	ofstream ostream(filename, ios::out);
	if (ostream.fail()) {
		cout << "File open failed!" << endl;
		return 0;
	}
	while (1) {
		cin >> p; // read fields of person
		if (strlen(p.id) == 0) break;
		// write person to output stream
		cout << p<< endl; // write fields of person
		ostream << p;
		cin >> C;
		if (strlen(C.course_id) == 0) break;
		cout << C << endl;
		ostream << C;
	}
	return 1;
}

