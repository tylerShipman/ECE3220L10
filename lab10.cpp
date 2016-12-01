nclude <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string>
#define MAX 5
using namespace std;

class Message {
protected:
	string message; //Holds message
public:
	Message();
	Message(string);
	virtual ~Message();
	virtual void printInfo(); //Needs to be redefined. Similar to abstract in java
};
Message::Message() {
	cout << "Please enter a message to convert into Morse code: " << endl;
	cin >> message;
}
Message::Message(string m) {
	message = m;
}
Message::~Message() {
}
void Message::printInfo() {
	cout << message << endl;
}
class MorseCodeMessage: public Message {
private:
//stores letters in morse code. index correlates to value - 97

public:
	MorseCodeMessage(string); //Message to translate
	~MorseCodeMessage();
	string * translatedMessage; //Stores translated message
	void translate(); //translates message
	void printInfo(); //prints trans msg

	int morseToLED();

};
MorseCodeMessage::MorseCodeMessage(string m) :
		Message(m) {
	translate();
}
MorseCodeMessage::~MorseCodeMessage() {
	delete[] translatedMessage;
}
void MorseCodeMessage::translate() {
	translatedMessage = new string[message.length()];
	string letterLookup[26] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
			"....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.",
			"--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--",
			"--.." };
//stores numbers in morse code. index correlates to value
	string numberLookup[10] = { "-----", ".----", "..---", "...--", "....-",
			".....", "-....", "--...", "---..", "----." };
	for (int i = 0; i < (int) message.length(); i++) {
		if (isalpha(message[i])) { //Check input is a letter
			translatedMessage[i] = letterLookup[tolower(message[i]) - 97]; //Subtract 97 to get index val from ascii value.
		} else if (isspace(message[i])) { // Check for space
			translatedMessage[i] = "/";
		} else if (isdigit(message[i])) { //check for number
			translatedMessage[i] = numberLookup[tolower(message[i]) - 48]; //sub 48 from ascii val to get index val
		} else {
			translatedMessage[i] = "#"; //For unrecognized chars i.e ?,#,$,%,^,etc.
		}
	}
}
void MorseCodeMessage::printInfo() {
	cout << message << endl;
	for (int i = 0; i < (int) message.length(); i++) {
		cout << translatedMessage[i] << " ";
	}
	cout << endl;
}

int MorseCodeMessage::morseToLED() {

	int fd;		// for the file descriptor of the special file we need to open.
	unsigned long *BasePtr;	// base pointer, for the beginning of the memory page (mmap)
	unsigned long *PBDR, *PBDDR;	// pointers for port B DR/DDR

	fd = open("/dev/mem", O_RDWR | O_SYNC);	// open the special file /dem/mem
	if (fd == -1) {
		printf("\n error\n");
		return (-1);  // failed open
	}

	// We need to map Address 0x80840000 (beginning of the page)
	BasePtr = (unsigned long*) mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0x80840000);
	if (BasePtr == MAP_FAILED) {
		printf("\n Unable to map memory space \n");
		return (-2);
	}  // failed mmap

	close(fd);

	// To access other registers in the page, we need to offset the base pointer to reach the
	// corresponding addresses. Those can be found in the board's manual.
	PBDR = BasePtr + 1;		// Address of port B DR is 0x80840004
	PBDDR = BasePtr + 5;	// Address of port B DDR is 0x80840014

	*PBDDR |= 0xE0;			// configures port B7 as output (Green LED)
	*PBDDR &= 0xFFFFFFFE;// configures port B0 as input (first push button). You could use: &= ~0x01
	*PBDR &= 0x00;
	// The program will turn on the green LED, sleep for a while, then off, sleep, then on again, then off.
	// You could use loops, if you wanted/needed.
	*PBDR |= 0x80;	// ON: write a 1 to port B0. Mask all other bits.


	sleep(1);	// How can you sleep for less than a second?
	*PBDR &= ~0x80;	// OFF: write a 0 to port B0. Mask all other bits.
	sleep(1);
	*PBDR |= 0x80;
	sleep(2);
	*PBDR &= ~0x80;

	// If you wanted to read the status of Port B0, how could you do it?



	// Message class default constructor asks user for input
	// morseCodeMessage does the translation
	// To verify this we can print the translated msg to stdout
	cout << "Original Message: " << message << endl;
	cout << "Morse code: ";
	for (int i = 0; i < message.length(); i++) {
		cout << translatedMessage[i];
	}
	cout << "\n";
	// Set pushbutton 1 as input, 3 LED outputs.
	//gpio_init();

	for (int i = 0; i < message.length(); i++) {
		string morse = translatedMessage[i];
		//for each char in string
		for (int j = 0; j < morse.length(); j++) {
			usleep(500000);
			char c = morse[j];
			if (c == '.') {
				// RED LED => B5
				// Turn on B5
				*PBDR |= 0x20;
				cout << "Red" << endl;
				// Delay 0.5 seconds between morse code characters
				usleep(500000);
				// Off
				*PBDR &= ~0x20;
			} else if (c == '-') {
				// YELLOW LED => B6
				// On
				*PBDR |= 0x40;
				cout << "Yellow" << endl;
				// Delay 0.5 seconds between morse code characters
				usleep(500000);
				// Off
				*PBDR &= ~0x40;
			} else {
				cout << "Character not morse code: " << c << endl;
			}
		}
		// Delay 1 second between ascii characters
		sleep(1);
	}
	// Pulse green LED to signify the end of the word
	// GREEN LED => B7
	// On
	*PBDR |= 0x80;
	cout << "Green" << endl;
	// Delay 1 second
	sleep(1);
	// Off
	*PBDR |= 0x80;
	// All LED off
	*PBDR &= 0x00;
}

class MessageStack {
public:
	MessageStack();
	MessageStack(Message*);
	~MessageStack();
	Message* ptrStack[MAX]; //Max size of 5
	int topOfStack; //Top of stack index
	Message* pop(); //Pops last LIFO
	void push(Message* object); //pushes last LIFO
	void printStack(); //prints contents
};
MessageStack::MessageStack() {
	topOfStack = 0; //initializes stack with size 0
}
MessageStack::MessageStack(Message* message) {
	topOfStack = 0;
	push(message);
}
MessageStack::~MessageStack() {
}
void MessageStack::push(Message* object) {
	if (topOfStack != MAX) { //pushes if stack has not reached max
		ptrStack[topOfStack] = object;
		topOfStack++;
	} else
		cout << "Stack is full" << endl;
}
Message* MessageStack::pop() {
	if (topOfStack == 0) {
		cout << "\nStack empty, can't pop any more" << endl;
		return NULL;
	} else {
		Message *hold = ptrStack[topOfStack];
		topOfStack--;
		return hold;
	}
}
void MessageStack::printStack() {
	cout << "Stack: " << endl;
	for (int i = 0; i < topOfStack; i++) {
		ptrStack[i]->printInfo();
	}
	cout << endl
			<< "___________________________________________________________________________________"
			<< endl << endl;
}

int main() {

	Message message = Message("Tyler");

	message.printInfo();
	MorseCodeMessage morse1 = MorseCodeMessage("Tyler");
	morse1.printInfo();
	morse1.morseToLED();

//	MorseCodeMessage msg1("Tyler123");
//	MorseCodeMessage msg2("Tyler Shipman");
//	MessageStack stack;
//
//	stack.push(&msg1);
//	stack.push(&msg2);
//
//	stack.printStack();
//	stack.pop();
//	stack.printStack();
//	stack.pop();
//	stack.printStack();
//	stack.pop();
//	stack.printStack();
	return 0;
}


