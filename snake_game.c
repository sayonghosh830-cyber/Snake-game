#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define WIDTH 20
#define HEIGHT 20
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

struct Node
{
    int x, y;
    struct Node *next;
};

struct Node *head = NULL;

int foodX, foodY;
int gameOver = 0;
int score = 0;
char direction = 'd';

// ---------- NON-BLOCKING INPUT ----------

int kbhit()       // Check key press
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char getch()        // Get character
{
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

// ---------- LINKED LIST ----------

struct Node *createNode(int x, int y)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    newNode->x = x;
    newNode->y = y;
    newNode->next = NULL;
    return newNode;
}

void addHead(int x, int y)
{
    struct Node *newNode = createNode(x, y);
    newNode->next = head;
    head = newNode;
}

void deleteTail()
{
    if (head == NULL || head->next == NULL)
        return;

    struct Node *temp = head;
    while (temp->next->next != NULL)
        temp = temp->next;

    free(temp->next);
    temp->next = NULL;
}

// ---------- GAME DESIGN ----------

void generateFood();

void setup()        // Place snake head and food 
{
    head = createNode(WIDTH / 2, HEIGHT / 2);
    generateFood();
}

int isOnSnake(int x, int y)
{
    struct Node *temp = head;
    while (temp != NULL)
    {
        if (temp->x == x && temp->y == y)
            return 1;
        temp = temp->next;
    }
    return 0;
}

void generateFood()         // Generate food coordinates
{
    do
    {
        foodX = rand() % WIDTH;
        foodY = rand() % HEIGHT;
    } while (isOnSnake(foodX, foodY));
}

void draw()    
{
    printf("\033[H\033[J");     // Clear screen

    for (int i = 0; i < WIDTH + 2; i++)     // Draw the top border
        printf("#");
    printf("\n");

    for (int i = 0; i < HEIGHT; i++)      // Draw the left border
    {
        printf("#");

        for (int j = 0; j < WIDTH; j++)
        {
            struct Node *temp = head;
            int printed = 0;

            while (temp != NULL)      // Draw the snake
            {
                if (temp->x == j && temp->y == i)
                {
                    if (temp == head)
                        printf(GREEN "O" RESET);
                    else
                        printf(CYAN "o" RESET);
                    printed = 1;
                    break;
                }
                temp = temp->next;
            }

            if (!printed)
            {
                if (i == foodY && j == foodX)    // Draw the food
                    printf(RED "*" RESET);
                else
                    printf(" ");
            }
        }

        printf("#\n");      // Draw the right border
    }

    for (int i = 0; i < WIDTH + 2; i++)     // Draw the bottom border
        printf("#");

    printf("\nScore: %d\n", score);
}

void input()    // Read the user input
{
    if (kbhit())
    {
        char ch = getch();

        if ((ch == 'w'||ch =='W') && direction != 's')
            direction = 'w';
        else if ((ch == 's'||ch=='S') && direction != 'w')
            direction = 's';
        else if ((ch == 'a'||ch=='A') && direction != 'd')
            direction = 'a';
        else if ((ch == 'd'||ch=='D') && direction != 'a')
            direction = 'd';
    }
}

void logic()      // Establishing game logic
{
    int newX = head->x;
    int newY = head->y;

    if (direction == 'w')
        newY--;
    else if (direction == 's')
        newY++;
    else if (direction == 'a')
        newX--;
    else if (direction == 'd')
        newX++;

    if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT)      // Wall collision
    {
        gameOver = 1;
        return;
    }

    struct Node *temp = head;     // Self collision
    while (temp != NULL)
    {
        if (temp->x == newX && temp->y == newY)
        {
            gameOver = 1;
            return;
        }
        temp = temp->next;
    }

    addHead(newX, newY);

    if (newX == foodX && newY == foodY)       // Logic when food is consumed
    {
        score += 10;
        generateFood();
    }
    else
    {
        deleteTail();
    }
}

int main()
{
    printf("\033[?25l"); // hide cursor
    setup();
    int delay = 500000; // initial speed

    while (!gameOver)
    {
        draw();
        input();
        logic();

        delay = 500000 - (score * 5000); // speed increases with score

        if (delay < 30000) // Minimum speed
            delay = 30000;

        usleep(delay);
    }

    printf("\nGAME OVER!\n");
    printf("Final Score: %d\n", score);
    
    printf("\033[?25h"); // show cursor again
    return 0;
}