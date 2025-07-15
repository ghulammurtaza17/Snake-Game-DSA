#include <iostream>
#include <windows.h>
#include <conio.h>
#include <queue>
#include <vector>
#include <ctime>

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 10;

enum Direction { STOP = -1, LEFT = 0, RIGHT = 1, UP = 2, DOWN = 3 };
Direction dir = STOP;

int dx[] = { -1,1,0,0 };
int dy[] = { 0,0,-1,1 };

bool gameOver = false;
bool autoMode = false;
bool obstacleGrid[WIDTH][HEIGHT] = { false };

int fruitX, fruitY;

struct SnakeNode {
	int x, y;
	SnakeNode* next;
	SnakeNode(int _x, int _y) :x(_x), y(_y), next(nullptr) {}
};

SnakeNode* head = nullptr;

void clearScreen() {
	COORD topLeft = { 0,0 };
	DWORD written;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	GetConsoleScreenBufferInfo(hOut, &screen);
	DWORD cells = screen.dwSize.X * screen.dwSize.Y;
	FillConsoleOutputCharacter(hOut, ' ', cells, topLeft, &written);
	SetConsoleCursorPosition(hOut, topLeft);
}

bool isBodycell(int x, int y) {
	SnakeNode* temp = head;
	while (temp != nullptr) {
		if (temp->x == x && temp->y == y)
			return true;
		temp = temp->next;
	}
	return false;
}

bool isObstacle(int x, int y) {
	return obstacleGrid[y][x];
}

void placeFruit() {
	do {
		fruitX = rand() % WIDTH;
		fruitY = rand() % HEIGHT;
	} while (isBodycell(fruitX, fruitY) || isObstacle(fruitX, fruitY));
}

void placeObstacles() {
	int obstacleCount = 10;
	while (obstacleCount--) {
		int x = rand() % WIDTH;
		int y = rand() % HEIGHT;
		if (!isBodycell(x, y) && !isObstacle(x, y)) {
			obstacleGrid[x][y] = true;
		}
	}
}

void draw() {
	clearScreen();
	for (int x = 0;x <= WIDTH + 1;x++) cout << "_";
	cout << "\n";

	for (int y = 0;y < HEIGHT;y++) {
		cout << "|";
		for (int x = 0;x < WIDTH;x++) {
			if (head->x == x && head->y == y) cout << "0";
			else if (fruitX == x && fruitY == y)cout << "*";
			else if (obstacleGrid[y][x]) cout << "#";
			else {
				bool isBody = false;
				SnakeNode* temp = head->next;
				while (temp != nullptr) {
					if (temp->x == x && temp->y == y) {
						isBody = true;
						break;
					}
					temp = temp->next;
				}
				cout << (isBody ? "o" : ".");
			}
		}
		cout << "|\n";
	}
	for (int x = 0;x <= WIDTH + 1;x++)cout << "-";
	cout << "\n";
}

bool isOpposite(Direction a, Direction b) {
	return (a == LEFT && b == RIGHT) || (a == RIGHT && b == LEFT) ||
		(a == UP && b == DOWN) || (a == DOWN && b == UP);
}

Direction getNextDirectionBFS() {
	struct Node {
		int x, y;
		vector<Direction> path;
	};
	bool visited[HEIGHT][WIDTH] = { false };
	queue<Node> q;

	q.push({ head->x, head->y,{} });
	visited[head->y][head->x] = true;

	int secondX = head->next ? head->next->x : -1;
	int secondY = head->next ? head->next->y : -1;

	while (!q.empty()) {
		Node current = q.front();q.pop();

		if (current.x == fruitX && current.y == fruitY && !current.path.empty()) {
			return current.path[0];
		}

		for (int i = 0;i < 4;i++) {
			int nx = current.x + dx[i];
			int ny = current.y + dy[i];

			if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT &&
				!visited[ny][nx] && !isBodycell(nx, ny) && !isObstacle(nx, ny)) {

				if (nx == secondX && ny == secondY) continue;

				visited[ny][nx] = true;
				vector<Direction> newPath = current.path;
				newPath.push_back(static_cast<Direction>(i));
				q.push({ nx,ny,newPath });
			}
		}
	}
	return STOP;
}

void moveSnake() {
	int nextX = head->x + dx[dir];
	int nextY = head->y + dy[dir];

	if (nextX < 0 || nextX >= WIDTH || nextY < 0 || nextY >= HEIGHT ||
		isBodycell(nextX, nextY) || isObstacle(nextX, nextY)) {
		gameOver = true;
		return;
	}

	SnakeNode* newHead = new SnakeNode(nextX, nextY);
	newHead->next = head;
	head = newHead;

	if (nextX == fruitX && nextY == fruitY) {
		placeFruit();
		return;
	}
	SnakeNode* prev = nullptr;
	SnakeNode* curr = head;
	while (curr->next != nullptr) {
		prev = curr;
		curr = curr->next;
	}
	delete curr;
	prev->next = nullptr;
}

void moveSnakeManual() {
	if (_kbhit()) {
		char ch = _getch();
		Direction newDir = dir;
		switch (ch) {
		case 'a': newDir = LEFT;break;
		case 'd': newDir = RIGHT;break;
		case 'w': newDir = UP;break;
		case 's': newDir = DOWN;break;
		}
		if (!isOpposite(dir, newDir) || dir == STOP)
			dir = newDir;
	}
	if (dir != STOP) {
		moveSnake();
	}
}

void moveSnakeAuto() {
	Direction nextDir = getNextDirectionBFS();

	if (nextDir == STOP) {
		gameOver = true;
		return;
	}
	if (isOpposite(dir, nextDir)) return;
	dir = nextDir;
	moveSnake();
}

int getSnakeLength() {
	int count = 0;
	SnakeNode* temp = head;
	while (temp) {
		count++;
		temp = temp->next;
	}
	return count;
}

int main() {
	srand(time(NULL));

	head = new SnakeNode(WIDTH / 2, HEIGHT / 2);
	cout << "------------------------------Wellcome To Snake Game Using BFS Traversal------------------------------" << endl;
	cout << endl;
	cout << "-------Select Mode-------\nTo Play Manual Press 1 & Enter\nTo Play Auto Using BFS With Obstacles Press 2 & Enter\n";
	int choice;
	cin >> choice;
	if (choice == 2) {
		autoMode = true;
		placeObstacles();
	}
	placeFruit();

	while (!gameOver) {
		draw();
		if (autoMode) moveSnakeAuto();
		else moveSnakeManual();
		Sleep(300);
	}

	cout << "\n Game Over! Final Score is: " << getSnakeLength() << "\n";
	return 0;
}