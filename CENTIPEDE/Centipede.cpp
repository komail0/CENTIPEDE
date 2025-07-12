#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;


const int resolutionX = 960;
const int resolutionY = 960;
const int boxPixelsX = 48;
const int boxPixelsY = 48;
const int gameRows = resolutionX / boxPixelsX; 
const int gameColumns = resolutionY / boxPixelsY; 


const int MAX_CENTIPEDES = 10;
const int MAX_SEGMENTS = 20;
float centipedes[MAX_CENTIPEDES][MAX_SEGMENTS * 3]; 
int centipedeSizes[MAX_CENTIPEDES] = { 0 };          
int centipedeDirections[MAX_CENTIPEDES] = { 0 };      
int numCentipedes = 1;                              
int level = 1;                                      
bool gameOver = false;                              
bool gameWon = false;                              
Sprite mushroomSprites[15];
Texture mushroomTexture;
bool mushroomInitialized = false;

int gameGrid[gameRows][gameColumns] = {};


const int x = 0;
const int y = 1;
const int exists = 2;

void drawPlayer(RenderWindow& window, float player[], Sprite& playerSprite) {
    playerSprite.setPosition(player[x], player[y]);
    window.draw(playerSprite);
}

void moveBullet(float bullet[], sf::Clock& bulletClock) {
    if (bulletClock.getElapsedTime().asMilliseconds() < 20)
        return;

    bulletClock.restart();
    bullet[y] -= 60;
    if (bullet[y] < 85)
        bullet[exists] = false;
}

void drawBullet(RenderWindow& window, float bullet[], Sprite& bulletSprite) {
    bulletSprite.setPosition(bullet[x], bullet[y]);
    window.draw(bulletSprite);
}

void drawMushrooms(RenderWindow& window, float bullet[], Sprite& bulletSprite, Text& score, int& playerScore, float mushroomPositions[][2], int mushroomHealth[]) {
   
    if (!mushroomInitialized) {
        if (!mushroomTexture.loadFromFile("Textures/mush.png")) {
            cout << "Failed to load mushroom texture" << endl;
            return;
        }

        for (int i = 0; i < 15; i++) {
            mushroomPositions[i][x] = (rand() % (gameColumns - 2) + 1) * boxPixelsX;
            mushroomPositions[i][y] = (rand() % (gameRows / 2)) * boxPixelsY + 100;
            mushroomHealth[i] = 2;
            mushroomSprites[i].setTexture(mushroomTexture);
            mushroomSprites[i].setTextureRect(IntRect(0, 0, 48, 47)); // Set full texture rectangle
            mushroomSprites[i].setPosition(mushroomPositions[i][x], mushroomPositions[i][y]);
        }
        mushroomInitialized = true;
    }

   
    for (int i = 0; i < 15; i++) {
        if (mushroomHealth[i] > 0) {
            
            if (bullet[exists] && bullet[y] < mushroomPositions[i][y] + boxPixelsY &&
                bullet[y] + boxPixelsY > mushroomPositions[i][y] &&
                bullet[x] > mushroomPositions[i][x] - boxPixelsX / 4 &&
                bullet[x] < mushroomPositions[i][x] + boxPixelsX) {
                mushroomHealth[i]--;
                bullet[exists] = false;
                if (mushroomHealth[i] == 1) {
                    mushroomSprites[i].setTextureRect(IntRect(0, 0, 25, 47)); // Half-damaged texture
                }
                else if (mushroomHealth[i] == 0) {
                    playerScore += 10;
                    score.setString("SCORE: " + to_string(playerScore));
                }
            }
            if (mushroomHealth[i] > 0) {
                window.draw(mushroomSprites[i]);
            }
        }
    }
}
void movePlayer(RenderWindow& window, float player[], Sprite& playerSprite) {
   
    if (Keyboard::isKeyPressed(Keyboard::A)) {
        if (player[x] > 15)
            player[x] -= boxPixelsX / 30;
    }
    if (Keyboard::isKeyPressed(Keyboard::D)) {
        if (player[x] < resolutionX - boxPixelsX - 25)
            player[x] += boxPixelsX / 30;
    }
    if (Keyboard::isKeyPressed(Keyboard::W)) {
        if (player[y] > 650)
            player[y] -= boxPixelsY / 30;
    }
    if (Keyboard::isKeyPressed(Keyboard::S)) {
        if (player[y] < resolutionY - boxPixelsY - 30)
            player[y] += boxPixelsY / 30;
    }
}

void drawCentipede(RenderWindow& window, float centipedes[MAX_CENTIPEDES][MAX_SEGMENTS * 3], int centipedeSizes[], Sprite& centipedeHeadSprite, Sprite& centipedeBodySprite) {
    for (int c = 0; c < numCentipedes; c++) {
        for (int i = 0; i < centipedeSizes[c]; i++) {
            if (centipedes[c][i * 3 + exists]) {
                if (i == 0) {
                    
                    centipedeHeadSprite.setPosition(centipedes[c][i * 3 + x], centipedes[c][i * 3 + y]);
                    window.draw(centipedeHeadSprite);
                }
                else {
                  
                    centipedeBodySprite.setPosition(centipedes[c][i * 3 + x], centipedes[c][i * 3 + y]);
                    window.draw(centipedeBodySprite);
                }
            }
        }
    }
}

void moveCentipede(float centipedes[MAX_CENTIPEDES][MAX_SEGMENTS * 3], int centipedeSizes[], int centipedeDirections[], int& numCentipedes, sf::Clock& centipedeClock, float bullet[], int& playerScore, Text& score, float player[], bool& gameOver, bool& gameWon) {
    if (centipedeClock.getElapsedTime().asMilliseconds() < 25)
        return;

    centipedeClock.restart();

    float spacingFactor = 0.5f;
    float headOffset = 10.0f;
    float newCentipedes[MAX_CENTIPEDES][MAX_SEGMENTS * 3] = { {0} };
    int newCentipedeSizes[MAX_CENTIPEDES] = { 0 };
    int newCentipedeDirections[MAX_CENTIPEDES] = { 0 };
    int newNumCentipedes = 0;
    bool allCentipedesDead = true;

    for (int c = 0; c < numCentipedes; c++) {
        if (centipedeSizes[c] == 0) continue;

        allCentipedesDead = false;
        float* segments = centipedes[c];
        int size = centipedeSizes[c];
        int direction = centipedeDirections[c];

        float prevHeadX = segments[x];
        float prevHeadY = segments[y];

        float& headX = segments[x];
        float& headY = segments[y];
        bool moveDown = false;
        bool inPlayerArea = headY >= 650;

        headX += direction * boxPixelsX / 4;

        if (headX >= resolutionX - boxPixelsX - 25 && direction == 1) {
            moveDown = true;
            direction = -1;
        }
        else if (headX <= 15 && direction == -1) {
            moveDown = true;
            direction = 1;
        }

        if (moveDown) {
            if (!inPlayerArea || (inPlayerArea && headY < resolutionY - boxPixelsY - 30)) {
                headY += boxPixelsY;
            }
            if (inPlayerArea && headY >= resolutionY - boxPixelsY - 30) {
                headY = 650;
            }
            else if (!inPlayerArea && headY >= 650) {
                headY = 650;
            }
        }

        for (int i = size - 1; i > 0; i--) {
            if (segments[i * 3 + exists]) {
                float targetX = segments[(i - 1) * 3 + x] - direction * (i == 1 ? spacingFactor + 0.5 : spacingFactor) * boxPixelsX;
                float targetY = segments[(i - 1) * 3 + y] + (i == 1 ? headOffset : 0);
                segments[i * 3 + x] = targetX;
                segments[i * 3 + y] = targetY;
                segments[i * 3 + exists] = segments[(i - 1) * 3 + exists];
            }
        }

        for (int i = 0; i < size; i++) {
            if (segments[i * 3 + exists]) {
                float segX = segments[i * 3 + x];
                float segY = segments[i * 3 + y];
                if (segY < player[y] + boxPixelsY &&
                    segY + boxPixelsY > player[y] &&
                    segX > player[x] - boxPixelsX / 4 &&
                    segX < player[x] + boxPixelsX) {
                    gameOver = true;
                    return;
                }
            }
        }

        bool splitOccurred = false;
        int splitIndex = -1;
        for (int i = 0; i < size; i++) {
            if (!segments[i * 3 + exists]) continue;
            float segX = segments[i * 3 + x];
            float segY = segments[i * 3 + y];
            if (bullet[exists] &&
                bullet[y] < segY + boxPixelsY &&
                bullet[y] + boxPixelsY > segY &&
                bullet[x] > segX - boxPixelsX / 4 &&
                bullet[x] < segX + boxPixelsX) {
                bullet[exists] = false;
                playerScore += 50;
                score.setString("SCORE: " + to_string(playerScore));

                if (i == 0) {
                    if (size > 1) {
                        centipedeSizes[c]--;
                        for (int j = 0; j < centipedeSizes[c]; j++) {
                            segments[j * 3 + x] = segments[(j + 1) * 3 + x];
                            segments[j * 3 + y] = segments[(j + 1) * 3 + y];
                            segments[j * 3 + exists] = segments[(j + 1) * 3 + exists];
                        }
                        segments[y] -= headOffset;
                    }
                    else {
                        segments[i * 3 + exists] = false;
                        centipedeSizes[c] = 0;
                    }
                    break;
                }
                else if (i > 0) {
                    splitIndex = i;
                    splitOccurred = true;
                    break;
                }
            }
        }

        if (splitOccurred && newNumCentipedes + 2 <= MAX_CENTIPEDES) {
            int firstSize = splitIndex;
            if (firstSize > 0) {
                for (int j = 0; j < firstSize; j++) {
                    newCentipedes[newNumCentipedes][j * 3 + x] = segments[j * 3 + x];
                    newCentipedes[newNumCentipedes][j * 3 + y] = segments[j * 3 + y];
                    newCentipedes[newNumCentipedes][j * 3 + exists] = segments[j * 3 + exists];
                }
                newCentipedeSizes[newNumCentipedes] = firstSize;
                newCentipedeDirections[newNumCentipedes] = direction;
                newNumCentipedes++;
            }

            int secondSize = size - splitIndex - 1;
            if (secondSize > 0) {
                for (int j = 0; j < secondSize; j++) {
                    newCentipedes[newNumCentipedes][j * 3 + x] = segments[(splitIndex + 1 + j) * 3 + x];
                    newCentipedes[newNumCentipedes][j * 3 + y] = segments[(splitIndex + 1 + j) * 3 + y];
                    newCentipedes[newNumCentipedes][j * 3 + exists] = segments[(splitIndex + 1 + j) * 3 + exists];
                }
                newCentipedes[newNumCentipedes][y] -= headOffset;
                newCentipedeSizes[newNumCentipedes] = secondSize;
                newCentipedeDirections[newNumCentipedes] = -direction;
                newNumCentipedes++;
            }
            centipedeSizes[c] = 0;
        }
        else if (centipedeSizes[c] > 0) {
            for (int j = 0; j < centipedeSizes[c]; j++) {
                newCentipedes[newNumCentipedes][j * 3 + x] = segments[j * 3 + x];
                newCentipedes[newNumCentipedes][j * 3 + y] = segments[j * 3 + y];
                newCentipedes[newNumCentipedes][j * 3 + exists] = segments[j * 3 + exists];
            }
            newCentipedeSizes[newNumCentipedes] = centipedeSizes[c];
            newCentipedeDirections[newNumCentipedes] = direction;
            newNumCentipedes++;
        }
    }

    for (int c = 0; c < MAX_CENTIPEDES; c++) {
        centipedeSizes[c] = newCentipedeSizes[c];
        centipedeDirections[c] = newCentipedeDirections[c];
        for (int j = 0; j < MAX_SEGMENTS * 3; j++) {
            centipedes[c][j] = newCentipedes[c][j];
        }
    }
    numCentipedes = newNumCentipedes;

    if (allCentipedesDead) {
        gameWon = true;
    }
}
void menu(RenderWindow& window, int& currentPage) {
    Sprite bg;
    Texture bgTexture;
    if (!bgTexture.loadFromFile("Textures/bg2.png")) {
        cout << "Failed to load menu background" << endl;
        return;
    }
    bg.setTexture(bgTexture);

    Sprite logo;
    Texture logoTexture;
    if (!logoTexture.loadFromFile("Textures/logo1.png")) {
        cout << "Failed to load logo" << endl;
        return;
    }
    logo.setTexture(logoTexture);
    logo.setPosition(30, 10);

    Sprite play;
    Texture playTexture;
    if (!playTexture.loadFromFile("Textures/play.png")) {
        cout << "Failed to load play button" << endl;
        return;
    }
    play.setTexture(playTexture);
    play.setPosition(50, 200);

    Sprite guide;
    Texture guideTexture;
    if (!guideTexture.loadFromFile("Textures/guide.png")) {
        cout << "Failed to load guide button" << endl;
        return;
    }
    guide.setTexture(guideTexture);
    guide.setPosition(500, 500);

    Sprite back;
    Texture backTexture;
    if (!backTexture.loadFromFile("Textures/back.png")) {
        cout << "Failed to load guide button" << endl;
        return;
    }
    back.setTexture(backTexture);
    back.setPosition(370, 850);

    Sprite quit;
    Texture quitTexture;
    if (!quitTexture.loadFromFile("Textures/quit.png")) {
        cout << "Failed to load quit button" << endl;
        return;
    }
    quit.setTexture(quitTexture);
    quit.setPosition(50, 700);

    Sprite inst;
    Texture instTexture;
    if (!instTexture.loadFromFile("Textures/instructions.png")) {
        cout << "Failed to load quit button" << endl;
        return;
    }
    inst.setTexture(instTexture);
    inst.setPosition(75, 200);


    while (window.isOpen()) {
       
           

            Event e;
            while (window.pollEvent(e)) {
                if (currentPage == 0) {
                    window.clear();
                    window.draw(bg);
                    window.draw(logo);
                    window.draw(play);
                    window.draw(guide);
                    window.draw(quit);
                    window.display();
                if (e.type == sf::Event::Closed) {
                    window.close();
                }

               
                if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                    if (play.getGlobalBounds().contains(mousePos)) {
                        currentPage = 2;
                        return;
                    }

                    if (guide.getGlobalBounds().contains(mousePos)) {
                        currentPage = 1;
                        break;
                    }

                    if (quit.getGlobalBounds().contains(mousePos)) {
                        window.close();
                        return;
                    }
                }
            }
                if (currentPage == 1) {
                    window.clear();
                    window.draw(bg);
                    window.draw(logo);
                    window.draw(inst);
                    window.draw(back);
                    window.display();
                    if (e.type == sf::Event::Closed) {
                        window.close();
                    }
                    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                        if (back.getGlobalBounds().contains(mousePos)) {
                            currentPage = 0;
                            return;
                        }

                    }
                }
        }
    }
  
}
void resetGame(float player[], float bullet[], float mushroomPositions[][2], int mushroomHealth[], int& playerScore, Text& score, sf::Clock& bulletClock, sf::Clock& centipedeClock) {
    player[x] = (gameColumns / 2) * boxPixelsX;
    player[y] = (gameColumns * 3 / 4) * boxPixelsY;

    bullet[x] = player[x] + boxPixelsX / 2;
    bullet[y] = player[y] - boxPixelsY;
    bullet[exists] = false;
    bulletClock.restart();

    numCentipedes = 1;
    for (int c = 0; c < MAX_CENTIPEDES; c++) {
        centipedeSizes[c] = 0;
        centipedeDirections[c] = 0;
        for (int i = 0; i < MAX_SEGMENTS * 3; i++) {
            centipedes[c][i] = 0;
        }
    }
    int startColumn = rand() % (gameColumns - 15) + 1;
    centipedeSizes[0] = 15;
    centipedeDirections[0] = 1;
    for (int i = 0; i < centipedeSizes[0]; i++) {
        centipedes[0][i * 3 + x] = (startColumn + i * 0.5f) * boxPixelsX;
        centipedes[0][i * 3 + y] = 90;
        centipedes[0][i * 3 + exists] = true;
    }
    centipedeClock.restart();

    
    for (int i = 0; i < 15; i++) {
        mushroomPositions[i][x] = (rand() % (gameColumns - 2) + 1) * boxPixelsX;
        mushroomPositions[i][y] = (rand() % (gameRows / 2)) * boxPixelsY + 100;
        mushroomHealth[i] = 2;
    }
    mushroomInitialized = false; 

    playerScore = 0;
    score.setString("SCORE: " + to_string(playerScore));
    gameOver = false;
    gameWon = false;
    level = 1;
}
int main() {
    srand(time(0));
    int currentPage = 0;

    
    RenderWindow window(VideoMode(resolutionX, resolutionY), "Centipede", Style::Close | Style::Titlebar);
    window.setSize(sf::Vector2u(960, 960));
    window.setPosition(sf::Vector2i(400, 0));

    SoundBuffer fireBuffer;
    if (!fireBuffer.loadFromFile("Textures/fire1.wav")) {
        cout << "Failed to load fire.wav" << endl;
       // return -1;
    }

    Sound fireSound;
    fireSound.setBuffer(fireBuffer);

    

  

    
    Music bgMusic;
    bgMusic.openFromFile("Textures/calm.ogg");
    bgMusic.play();
    bgMusic.setVolume(50);

   
    int playerScore = 0;
    Font font;
    font.loadFromFile("Textures/Congenial_Black.otf");
    Text score;
    score.setFont(font);
    score.setCharacterSize(70);
    score.setFillColor(Color(200, 200, 200));
    score.setOutlineColor(Color::Black);
    score.setOutlineThickness(4);
    score.setPosition(300, 10);
	score.setStyle(Text::Italic);
    score.setString("SCORE: " + to_string(playerScore));



   
    Texture backgroundTexture;
    Sprite backgroundSprite;
    backgroundTexture.loadFromFile("Textures/bg1.png");
    backgroundSprite.setTexture(backgroundTexture);

   
    float player[2] = {};
    player[x] = (gameColumns / 2) * boxPixelsX;
    player[y] = (gameColumns * 3 / 4) * boxPixelsY;
    Texture playerTexture;
    Sprite playerSprite;
    playerTexture.loadFromFile("Textures/player2.png");
    playerSprite.setTexture(playerTexture);

   
    float bullet[3] = {};
    bullet[x] = player[x] + boxPixelsX / 2;
    bullet[y] = player[y] - boxPixelsY;
    bullet[exists] = false;
    Clock bulletClock;
    Texture bulletTexture;
    Sprite bulletSprite;
    bulletTexture.loadFromFile("Textures/bullet1.png");
    bulletSprite.setTexture(bulletTexture);

    float mushroomPositions[15][2];
    int mushroomHealth[15];

   
    RectangleShape Area(Vector2f(930.0f, 850.0f));
    Area.setPosition(15, 100);
    Area.setFillColor(Color(255, 255, 255, 255 * 0.30));
    Area.setOutlineColor(Color::White);
    Area.setOutlineThickness(5);

    RectangleShape playArea(Vector2f(930.0f, 300.0f));
    playArea.setPosition(15, 650);
    playArea.setFillColor(Color(255, 0, 0, 255 * 0.30));
    playArea.setOutlineColor(Color::White);
    playArea.setOutlineThickness(5);

    Sprite win;
    Texture winTexture;
    if (!winTexture.loadFromFile("Textures/win.png")) {
        cout << "Failed to load menu background" << endl;
        return 0;
    }
    win.setTexture(winTexture);
    win.setPosition(190, 100);
    Sprite over;
    Texture overTexture;
    if (!overTexture.loadFromFile("Textures/over.png")) {
        cout << "Failed to load menu background" << endl;
        return 0;
    }
    over.setTexture(overTexture);
    over.setPosition(190, 100);

    Sprite main;
    Texture mainTexture;
    if (!mainTexture.loadFromFile("Textures/main.png")) {
        cout << "Failed to load menu background" << endl;
        return 0;
    }
    main.setTexture(mainTexture);
    main.setPosition(160, 680);

    Sprite quit;
    Texture quitTexture;
    if (!quitTexture.loadFromFile("Textures/quit1.png")) {
        cout << "Failed to load quit button" << endl;
        return 0;
    }
    quit.setTexture(quitTexture);
    quit.setPosition(160, 810);

    
    int startColumn = rand() % (gameColumns - 15) + 1; 
    centipedeSizes[0] = 15; 
    centipedeDirections[0] = 1; 
    for (int i = 0; i < centipedeSizes[0]; i++) {
        centipedes[0][i * 3 + x] = (startColumn + i * 0.5f) * boxPixelsX;
        centipedes[0][i * 3 + y] = 90;
        centipedes[0][i * 3 + exists] = true;
    }
    numCentipedes = 1;
    Clock centipedeClock;
    Texture centipedeHeadTexture, centipedeBodyTexture;
    Sprite centipedeHeadSprite, centipedeBodySprite;
    centipedeHeadTexture.loadFromFile("Textures/head.png");
    centipedeBodyTexture.loadFromFile("Textures/body.png");
    centipedeHeadSprite.setTexture(centipedeHeadTexture);
    centipedeBodySprite.setTexture(centipedeBodyTexture);

    while (window.isOpen()) {
        
        if(currentPage == 0) {
            menu(window,currentPage);
          
        }
        if (currentPage == 2) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == sf::Event::Closed) {
                    return 0;
                }
                if (!gameOver && !gameWon && Keyboard::isKeyPressed(Keyboard::Space) && bullet[exists] == false) {
                    bullet[x] = player[x] + boxPixelsX / 3;
                    bullet[y] = player[y] - boxPixelsY;
                    bullet[exists] = true;
                    fireSound.play();
                }
            }

           
            window.draw(backgroundSprite);
            window.draw(Area);
            window.draw(playArea);
            window.draw(score);

            if (!gameOver && !gameWon) {
                movePlayer(window, player, playerSprite);
                drawPlayer(window, player, playerSprite);
                drawMushrooms(window, bullet, bulletSprite, score, playerScore, mushroomPositions, mushroomHealth);
                moveCentipede(centipedes, centipedeSizes, centipedeDirections, numCentipedes, centipedeClock, bullet, playerScore, score, player, gameOver, gameWon);
                drawCentipede(window, centipedes, centipedeSizes, centipedeHeadSprite, centipedeBodySprite);
                if (bullet[exists] == true) {
                    moveBullet(bullet, bulletClock);
                    drawBullet(window, bullet, bulletSprite);
                }
            }

            
            if (gameOver) {
                window.draw(over);
                window.draw(quit);
                window.draw(main);
                if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                    if (main.getGlobalBounds().contains(mousePos)) {
                        resetGame(player, bullet, mushroomPositions, mushroomHealth, playerScore, score, bulletClock, centipedeClock);
                        currentPage = 0;  
                        
                    }

                    if (quit.getGlobalBounds().contains(mousePos)) {
                        window.close();

                       
                    }
                }
            }
            else if (gameWon) {
                window.draw(win);
                window.draw(quit);
                window.draw(main);
                if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                    if (main.getGlobalBounds().contains(mousePos)) {
                       
                        resetGame(player, bullet, mushroomPositions, mushroomHealth, playerScore, score, bulletClock, centipedeClock);
                        currentPage = 0;
                    
                    
                    }

                    if (quit.getGlobalBounds().contains(mousePos)) {
                        window.close();


                    }
                }
            }

            window.display();
            window.clear();
        }

    }
    return 0;
}