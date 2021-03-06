#include "State.h"

State::State(sf::RenderWindow* window, stack<State*>* states)
{
	this->window = window;
    this->states = states;
	this->quit = false;
    this->pause = false;
}

State::~State()
{
}

const bool& State::getQuit() const
{
	return this->quit;
}

void State::setQuit(bool q)
{
    this->quit = q;
}

void State::checkForQuit()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
		this->quit = true;
	}
}

//Pause and Unpause
void State::pauseState()
{
    this->pause = true;
}

void State::unpauseState()
{
    this->pause = false;
}

//GAMESTATE
GameState::GameState(sf::RenderWindow* window, stack<State*>* states) :
	State(window, states)
{
    initTextures();
    initFont();
    initPlayer();
    initEnemies();
    initBackground();
    initLevel();
    initTrafficLights();
    initSound();
    this->pauseMenu = new PauseMenu(this->window);
    this->messageBox = new MessageBox(this->window, this->points);
    this->points = 0;
    this->times = 0;
    this->enemySpawnTimerMax = 120.f;
    this->enemySpawnTimer = this->enemySpawnTimerMax;
    this->maxEnemies = 7;
    //this->backgroundSound.play();
}

GameState::GameState(sf::RenderWindow* window, stack<State*>* states, int level, int ipoint):
    State(window, states)
{
    initTextures();
    initFont();
    initPlayer();
    initEnemies();
    initBackground();
    initLevel(level);
    initTrafficLights();
    initSound();
    this->pauseMenu = new PauseMenu(this->window);
    this->messageBox = new MessageBox(this->window, this->points);
    this->points = ipoint;
    this->times = 0;
    this->enemySpawnTimerMax = 120.f;
    this->enemySpawnTimer = this->enemySpawnTimerMax;
    this->maxEnemies = 7;

    levelLabel = "Level: ";
    levelLabel += to_string(level);
    levelLabel += "          Score: ";
    levelLabel += to_string(points);
    levelLabelText.setString(levelLabel);
}

GameState::~GameState()
{
    int n = enemies.size();
    for (int i = 0; i < n; i++)
        delete enemies[i];
    n = trafficLights.size();
    for (int i = 0; i < n; i++)
        delete trafficLights[i];
    delete[] line;
}

void GameState::endState()
{   
    cout << "End gamestate" << endl;
}

void GameState::update()
{
    checkForPause();
    checkImpact();
    if (!this->pause) {
        this->checkForQuit();
        this->updateEnemies();
        this->updateTrafficLights();
        people.update();
        this->times ++;
    }
    else {
        if (!this->messageBox->pause) {
            pauseMenu->update();
            if (pauseMenu->getPause()) {
                this->pause = false;
                pauseMenu->setPause(false);
            }
        }
    }
    checkFromPause();
}

void GameState::saveFile() {
    ofstream fout;
    ofstream file_info;
    time_t now = time(0);
    tm* ltm = localtime(&now);

    int year = 1900 + ltm->tm_year;
    int month = 1 + ltm->tm_mon;
    int day = ltm->tm_mday;
    int hour = ltm->tm_hour;
    int minute = ltm->tm_min;
    string batch = to_string(year) + "_" + to_string(month) + "_" + to_string(day) + "_" + to_string(hour) + "_" + to_string(minute) + "_user";

    ifstream fin;
    fin.open("data/inf.txt");

    if (fin.is_open()) {
        for (int i = 0; i < 10; i++) {
            if (fin.eof()) break;
            string tmp;
            getline(fin, tmp);
            Lfile.push_back(tmp);
        }
    }
    fin.close();

    file_info.open("data/inf.txt", ostream::out);
    if (file_info.is_open()) {
        file_info << batch << endl;
        int size = (Lfile.size() >= 10 ? 10 : Lfile.size());
        for (int i = 0; i < size; i++) {
            file_info << Lfile[i] << endl;
        }
    }
    file_info.close();

    fout.open("data/" + batch + ".txt");
    if (fout.is_open()) {
        fout << currentLevel << " " << points << endl;
    }
    fout.close();
    cout << batch << endl;
}

void GameState::render(sf::Event &ev, sf::RenderTarget* target)
{   
    if (!target)
        target = this->window;
    target->draw(background);
    generateMap();
    target->draw(levelLabelText);
    renderPlayer(ev);
    renderEnemies();
    renderTrafficLights();
   
    if (this->pause) {
        //pause render
        if (this->messageBox->pause) {
            messageBox->setPoints(this->points);
            messageBox->draw(this->window);
            if (messageBox->checkQuit(ev)) {
                setQuit(true);
            }
        }
        else
            pauseMenu->render(ev, this->window);
    }
}

void GameState::initPlayer()
{
    this->people = PEOPLE(400, 500, &this->textures["people"]);
}

void GameState::initEnemies() {}

void GameState::initTextures()
{
    sf::Texture tmp;

    tmp.loadFromFile("sprites/car.png");
    tmp.setSmooth(true);
    this->textures["car"] = tmp;

    tmp.loadFromFile("sprites/bird.png");
    tmp.setSmooth(true);
    this->textures["bird"] = tmp;

    tmp.loadFromFile("sprites/truck.png");
    tmp.setSmooth(true);
    this->textures["truck"] = tmp;

    tmp.loadFromFile("sprites/dino.png");
    tmp.setSmooth(true);
    this->textures["dino"] = tmp;

    tmp.loadFromFile("sprites/player3.png");
    tmp.setSmooth(true);
    this->textures["people"] = tmp;

    tmp.loadFromFile("trafficLights/red.jfif");
    tmp.setSmooth(true);
    this->textures["red"] = tmp;

    tmp.loadFromFile("trafficLights/yellow.jpg");
    tmp.setSmooth(true);
    this->textures["yellow"] = tmp;

    tmp.loadFromFile("trafficLights/blue.jpg");
    tmp.setSmooth(true);
    this->textures["blue"] = tmp;

    tmp.loadFromFile("sprites/death.png");
    tmp.setSmooth(true);
    this->textures["people_dead"] = tmp;
}

void GameState::initBackground()
{
    this->background.setSize(sf::Vector2f((float)this->window->getSize().x, (float)this->window->getSize().y));
    if (!this->backgroundTexture.loadFromFile("images/bg1.png")) {
        throw "Texture load fail!! \n";
    }
    this->background.setTexture(&this->backgroundTexture);
}


void GameState::initLevel() {
	MAX_LEVEL = 3;
    levelLabel = "Level: 1";
    levelLabelText.setFont(font);
    levelLabelText.setString(levelLabel);
    levelLabelText.setFillColor(sf::Color(255, 255, 255, 255));
    levelLabelText.setPosition(50.f, 10.f);
    levelLabelText.setCharacterSize(30);
    levelLabelText.setStyle(sf::Text::Bold);
	setLevel(1);
}

void GameState::initLevel(int level) {
    MAX_LEVEL = 3;
    if (level > MAX_LEVEL) setLevel(1);
    else setLevel(level);

    levelLabel = "Level: ";
    levelLabel += to_string(level);
    levelLabelText.setFont(font);
    levelLabelText.setString(levelLabel);
    levelLabelText.setFillColor(sf::Color(255, 255, 255, 255));
    levelLabelText.setPosition(10.f, 10.f);
    levelLabelText.setCharacterSize(30);
    levelLabelText.setStyle(sf::Text::Bold);
}

void GameState::initTrafficLights() {
    currentTrafficLights = 3;
    this->trafficLightsSpawnTimer = 120.f;
    this->trafficLightsSpawnTimerMax = this->trafficLightsSpawnTimer;
    COBJECT* tmp = new CTRUCK(&this->textures["red"]);
    tmp->setPosition(550.f, 40.f);
    tmp->setScale(0.21f, 0.21f);
    tmp->setColor(0.f, 0.f, 0.f);
    trafficLights.push_back(tmp);
    tmp = new CTRUCK(&this->textures["yellow"]);
    tmp->setPosition(602.5f, 40.f);
    tmp->setScale(0.07f, 0.07f);
    tmp->setColor(0.f, 0.f, 0.f);
    trafficLights.push_back(tmp);
    tmp = new CTRUCK(&this->textures["blue"]);
    tmp->setPosition(650.f, 40.f);
    tmp->setScale(0.11f, 0.125f);
    tmp->setColor(0.f, 0.f, 0.f);
    trafficLights.push_back(tmp);
}

void GameState::initSound()
{
    GameOverSoundBuffer.loadFromFile("sound/GameOverSound.wav");
    GameOverSound.setBuffer(GameOverSoundBuffer);
}

void GameState::initFont()
{
    if (!this->font.loadFromFile("font/Dosis-Light.ttf")) {
        throw("Font not found! \n");
    }
}

void GameState::setLevel(unsigned level) {
    /*if (level > MAX_LEVEL)
        return;*/
    currentLevel = level;

    //this->points = 0;
    this->enemySpawnTimerMax = (140.f - static_cast<float>((level - 1) * 40));
    this->enemySpawnTimer = this->enemySpawnTimerMax;
    this->maxEnemies = 15;// loop some   times

    levelLabel = "Level: ";
    levelLabel += to_string(level);
    levelLabel += "          Score: ";
    levelLabel += to_string(points);
    levelLabelText.setString(levelLabel);
}

void GameState::updateEnemies() {
    if (this->enemies.size() < this->maxEnemies) {
        if (this->enemySpawnTimer >= this->enemySpawnTimerMax) {
            //spaw the enemy and reset the timer
            this->spawnEnemy();
            int keepSize = this->enemies.size();
            for (int i = 0; i < this->currentLevel; i++) {
                this->spawnEnemy();
                int keepCurrentSize = this->enemies.size();
                float checkSameLine = this->enemies[static_cast<int>(keepCurrentSize - 1)]->Y();
                for (unsigned j = keepSize; j < keepCurrentSize; j++) {
                    float checkSameLine1 = this->enemies[j - 1]->Y();
                    if (checkSameLine == checkSameLine1 || (checkSameLine == checkSameLine1 - 20) ||
                        checkSameLine == checkSameLine1 + 20) {
                        this->enemies.pop_back();
                        break;
                    }
                }
                if (this->enemies.size() == keepCurrentSize) {
                    this->enemies[this->enemies.size() - 1]->
                        setPosition(-100.f, this->enemies[this->enemies.size()-1]->Y());
                }
            }
            enemySpawnTimer = 0.f;
        }
        else
            enemySpawnTimer += 1.f;
    }
    else {
        enemies.erase(enemies.begin());// 6 = max - 1
    }
    for (auto& e : this->enemies) 
        e->update();
}

void GameState::renderEnemies() {

    for (auto& e : this->enemies) {
        e->Draw(this->window);
    }
    if (!this->pause) {
        for (auto& e : this->enemies) {//red, yellow, green
            if (typeid(*e) == typeid(CTRUCK) || typeid(*e) == typeid(CCAR)) {
                if (currentTrafficLights == 2)
                    e->Move(((3.f + (static_cast<float>(currentLevel - 1))) / 2) / 2, 0.f);
                else if (currentTrafficLights == 1)
                    e->Move(0.f, 0.f);
                else
                    e->Move((3.f + (static_cast<float>(currentLevel - 1))) / 2, 0.f);
            }
            else 
              e->Move((3.f + (static_cast<float>(currentLevel - 1))) / 2, 0.f);
        }
    }
}


void GameState::renderPlayer(sf::Event &ev)
 {
    this->people.Draw(this->window);
    if (!this->pause) {
        this->people.KeyBoadMove_WithDt(46.f, ev);
    }
}

void GameState::updateTrafficLights() {
    if (this->trafficLightsSpawnTimer < this->trafficLightsSpawnTimerMax)
        this->trafficLightsSpawnTimer++;
    else {
        if (currentTrafficLights <= 1)
            currentTrafficLights = 3;
        else
            currentTrafficLights--;
        if (currentTrafficLights == 1) {
            this->trafficLights[1]->setColor(0.f, 0.f, 0.f);
            this->trafficLights[0]->setColor(255.f, 255.f, 255.f);//red
        }
        else if (currentTrafficLights == 2) {
            this->trafficLights[2]->setColor(0.f, 0.f, 0.f);
            this->trafficLights[1]->setColor(255.f, 255.f, 0.f);//yellow
        }
        else {
            this->trafficLights[0]->setColor(0.f, 0.f, 0.f);
            this->trafficLights[2]->setColor(0.f, 255.f, 0.f);
        }
        this->trafficLightsSpawnTimer = 0.f;
    }
}

void GameState::renderTrafficLights(){
    for (auto& e : this->trafficLights)
        e->Draw(this->window);
}


void GameState::spawnEnemy() {
    COBJECT* enemyTmp = nullptr;
    unsigned tmp = static_cast<unsigned>(rand() % 4);
    float tmpp = 130 + static_cast<float>((rand() % 4) * 92);//set location
    this_thread::sleep_for(std::chrono::milliseconds(10));
    if (currentTrafficLights == 3) {
        switch (tmp)
        {
        case 0:
            enemyTmp = new CTRUCK(&this->textures["truck"]);
            break;
        case 1:
            enemyTmp = new CCAR(&this->textures["car"]);
            break;
        case 2:
            enemyTmp = new CBIRD(&this->textures["bird"]);
            break;
        default:
            enemyTmp = new CDINOSAUR(&this->textures["dino"]);
            break;
        }
        if (typeid(*enemyTmp) == typeid(CTRUCK) || typeid(*enemyTmp) == typeid(CBIRD))
            tmpp -= 20;
        enemyTmp->setPosition(0.f, tmpp);
        this->enemies.push_back(enemyTmp);
    }
    else if (currentTrafficLights == 2) {
        tmp = static_cast<unsigned>(rand() % 2);
        switch (tmp)
        {
        case 0:
            enemyTmp = new CCAR(&this->textures["car"]);
            break;
        default:
            enemyTmp = new CTRUCK(&this->textures["truck"]);
            break;
        }
        
        if (typeid(*enemyTmp) == typeid(CTRUCK) || typeid(*enemyTmp) == typeid(CBIRD))
            tmpp -= 20;
        enemyTmp->setPosition(0.f, tmpp);
        this->enemies.push_back(enemyTmp);
    }
    else {
        enemyTmp = new CBIRD(&this->textures["bird"]);
        if (typeid(*enemyTmp) == typeid(CTRUCK) || typeid(*enemyTmp) == typeid(CBIRD))
            tmpp -= 20;
        enemyTmp->setPosition(0.f, tmpp);
        this->enemies.push_back(enemyTmp);
    }
    
}
void GameState::generateMap()
{
    if (people.position().y < 20.f) {//change from 100 to 552 because the sprite is something mysterious about the location
        points += (500 - times);
        times = 0;
        if (currentLevel >= 3)
            setLevel(1);
        else {
            ++currentLevel;
            setLevel(currentLevel);
        }
        people.setPosition(400, 508);
        enemies.clear();
    }
}

void GameState::checkImpact()
{
    if (this->people.isImpact(this->enemies)) {
        GameOverSound.play();
        int x = people.position().x, y = people.position().y;
        people = PEOPLE(x, y, &this->textures["people_dead"]);
        this->pause = true;
        this->messageBox->pause = true;
    }
}

void GameState::checkForPause()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
        this->pause = true;
    }
}

void GameState::checkFromPause()
{
    if (this->pauseMenu->senderFromGame == 1) {
        saveFile();
        setQuit(true);
    }
    if (this->pauseMenu->senderFromGame == 2) {
        setQuit(true);
    }
}

//MENU STATE

MenuState::MenuState(sf::RenderWindow* window, stack<State*>* states) :
    State(window, states)
{
    initFont();
    initButton();
    initBackground();
    ismoving = false;
}

void MenuState::initButton()
{
    currentButton = 0;
    this->rec[0].setSize(sf::Vector2f(200, 75));
    this->rec[1].setSize(sf::Vector2f(200, 75));
    this->rec[2].setSize(sf::Vector2f(200, 75));

    this->rec[0].setFillColor(sf::Color(255, 0, 0, 255));
    this->rec[1].setFillColor(sf::Color(0, 255, 0, 255));
    this->rec[2].setFillColor(sf::Color(0, 0, 255, 255));

    this->rec[0].setPosition(sf::Vector2f(300, 100));
    this->rec[1].setPosition(sf::Vector2f(300, 225));
    this->rec[2].setPosition(sf::Vector2f(300, 350));

    this->text[0].setString("NEW GAME");
    this->text[1].setString("LOAD GAME");
    this->text[2].setString("QUIT NOW");


    for (int i = 0; i < 3; i++) {
        text[i].setFillColor(sf::Color(255, 255, 255, 255));
        rec[i].setOutlineColor(sf::Color(255, 255, 255));
        text[i].setPosition(
            rec[i].getPosition().x + 40 - text[i].getLocalBounds().width / 2.f,
            rec[i].getPosition().y + 15 - text[i].getLocalBounds().height / 2.f
        );
        text[i].setCharacterSize(30);
        text[i].setFont(this->font);
        text[i].setStyle(sf::Text::Bold);
    }
}

void MenuState::initBackground() {
    this->background.setSize(sf::Vector2f((float)this->window->getSize().x, (float)this->window->getSize().y));
    if (!this->backgroundTexture.loadFromFile("images/bg1.png")) {
        throw "Texture load fail!! \n";
    }
    this->background.setTexture(&this->backgroundTexture);
}

MenuState::~MenuState()
{
    delete this->window;
}

void MenuState::endState()
{   
    cout << "End GameState" << endl;
}


void MenuState::nextButton(sf::Event ev)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        ismoving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        ismoving = true;
    }

    if (ev.type == sf::Event::KeyReleased) {
        if (ev.key.code == sf::Keyboard::S && ismoving) {
            this->currentButton = (this->currentButton + 1) % 3;
            ismoving = false;
        }
        if (ev.key.code == sf::Keyboard::W && ismoving){
            this->currentButton = (this->currentButton + 2) % 3;
            ismoving = false;
        }
    }
}

void MenuState::highlight()
{
    for (int i = 0; i < 3; i++) 
        if (i != currentButton) 
            rec[i].setOutlineThickness(0);
        else 
            rec[i].setOutlineThickness(10);
}

void MenuState::initFont()
{
    if (!this->font.loadFromFile("font/Dosis-Light.ttf")) {
        throw("Font not found! \n");
    }
}


void MenuState::checkButton(sf::Event &ev)
{
    if (ev.key.code == sf::Keyboard::Enter) {
        if(currentButton == 0)
            this->states->push(new GameState(this->window, this->states));
        if (currentButton == 1) {
            this->states->push(new LoadFileState(this->window,this->states));
        }
        if (currentButton == 2)
            window->close();
    }
    ev.key.code = sf::Keyboard::Unknown;
}

void MenuState::update()
{
    highlight();
}

void MenuState::render(sf::Event &ev, sf::RenderTarget* target)
{
    nextButton(ev);
    if (!target)
        target = this->window;
    target->draw(background);
    for (int i = 0; i < 3; i++) {
        target->draw(rec[i]);
        target->draw(text[i]);
    }
    checkButton(ev);
}

LoadFileState::LoadFileState(sf::RenderWindow* window, stack<State*>* states) :
    State(window, states)
{
    initFont();
    initButton();
    initBackground();
}

vector<string> LoadFileState::getListfromFile() {
    vector<string> rs;
    ifstream fin;
    fin.open("data/inf.txt");
    if (fin.is_open()) {
        for (int i = 0; i < NUMBER_FILE; i++) {
            if (fin.eof()) break;
            string tmp;
            getline(fin, tmp);
            rs.push_back(tmp);
        }
    }
    fin.close();
    return rs;
}

void LoadFileState::initButton()
{
    currentButton = 0;
    for (int i = 0; i < NUMBER_FILE; i++) {
        this->rec[i].setSize(sf::Vector2f(150, 30));
        this->rec[i].setFillColor(sf::Color(160, 167, 136));
        this->rec[i].setPosition(sf::Vector2f(300, (i * 50) + 30));
    }


    vector<string> lFile = getListfromFile();
    for (int i = 0; i < NUMBER_FILE; i++) {
        this->text[i].setString(lFile[i]);
    }


    for (int i = 0; i < NUMBER_FILE; i++) {
        text[i].setFillColor(sf::Color(255, 255, 255, 255));
        rec[i].setOutlineColor(sf::Color(255, 255, 255));
        text[i].setPosition(
            rec[i].getPosition().x + 40 - text[i].getLocalBounds().width / 2.f,
            rec[i].getPosition().y + 15 - text[i].getLocalBounds().height / 2.f
        );
        text[i].setCharacterSize(10);
        text[i].setFont(this->font);
        text[i].setStyle(sf::Text::Bold);
    }

 
}

void LoadFileState::initBackground() {
    this->background.setSize(sf::Vector2f((float)this->window->getSize().x, (float)this->window->getSize().y));
    if (!this->backgroundTexture.loadFromFile("images/bg1.png")) {
        throw "Texture load fail!! \n";
    }
    this->background.setTexture(&this->backgroundTexture);
}

LoadFileState::~LoadFileState()
{
    delete this->window;
}

void LoadFileState::endState()
{
    cout << "End GameState" << endl;
}


void LoadFileState::nextButton(sf::Event ev)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        ismoving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        ismoving = true;
    }

    if (ev.type == sf::Event::KeyReleased) {
        if (ev.key.code == sf::Keyboard::S && ismoving) {
            this->currentButton = (this->currentButton + 1) % (NUMBER_FILE);
            ismoving = false;
        }
        if (ev.key.code == sf::Keyboard::W && ismoving) {
            if (this->currentButton == 0) 
                this->currentButton = NUMBER_FILE-1;
            else
                this->currentButton = (this->currentButton - 1) % (NUMBER_FILE);
            ismoving = false;
        }
    }
}

void LoadFileState::highlight()
{
    for (int i = 0; i < NUMBER_FILE; i++)
        if (i != currentButton)
            rec[i].setOutlineThickness(0);
        else
            rec[i].setOutlineThickness(10);
}

void LoadFileState::initFont()
{
    if (!this->font.loadFromFile("font/Dosis-Light.ttf")) {
        throw("Font not found! \n");
    }
}


void LoadFileState::checkButton()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
        if (currentButton >= 0 && currentButton < NUMBER_FILE) {
            string tmp = text[currentButton].getString();
            ifstream fin;
            if (!tmp.empty()) {
                int level;
                int ipoint;
                fin.open("data/" + tmp + ".txt");
                if (fin.is_open()) {
                    fin >> level;
                    fin >> ipoint;
                }
                fin.close();
                this->states->push(new GameState(this->window, this->states, level,ipoint));
            }
            else {
                this->states->push(new GameState(this->window, this->states));
            }
        }
    } 
}

void LoadFileState::update()
{
    checkButton();
    highlight();
    checkForQuit();
}

void LoadFileState::render(sf::Event& ev, sf::RenderTarget* target)
{
    nextButton(ev);
    if (!target)
        target = this->window;
    target->draw(background);
    for (int i = 0; i < NUMBER_FILE; i++) {
        target->draw(rec[i]);
        target->draw(text[i]);
    }


}
