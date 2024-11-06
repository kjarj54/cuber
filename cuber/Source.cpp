#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Dijkstra.h"
#include <string>
#include <deque>
#include <algorithm>

using namespace std;

void loadVerticesFromFile(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int id;
        float x, y;
        char dot;
        if (!(iss >> id >> dot >> x >> y)) {
            cerr << "Error al leer la línea: " << line << endl;
            continue;
        }
        string nodeName = "Node" + to_string(id);
        graph.addNode(nodeName, x, y);
    }
    file.close();
}

void loadGraphFromFile(Graph& graph, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Punto") != std::string::npos) {
            size_t startPos = line.find(" ") + 1;
            size_t endPos = line.find(":");
            int id;
            try {
                id = std::stoi(line.substr(startPos, endPos - startPos));
            }
            catch (const std::exception& e) {
                std::cerr << "Error al convertir el ID del punto: " << e.what() << std::endl;
                continue;
            }
            std::string nodeName = "Node" + std::to_string(id);

            std::getline(file, line);

            std::getline(file, line);
            size_t start = line.find(":") + 1;

            while ((start = line.find('(', start)) != std::string::npos) {
                size_t end = line.find(')', start);
                if (end == std::string::npos) break;

                std::string connection = line.substr(start + 1, end - start - 1);

                int neighborId;
                std::string direction;
                try {
                    size_t commaPos = connection.find(',');
                    if (commaPos != std::string::npos) {
                        neighborId = std::stoi(connection.substr(0, commaPos));
                        direction = connection.substr(commaPos + 1);

                        direction.erase(std::remove(direction.begin(), direction.end(), ' '), direction.end());
                        bool isBidirectional = (direction == "True");

                        std::string neighborName = "Node" + std::to_string(neighborId);
                        graph.addEdge(nodeName, neighborName, 1.0, isBidirectional);

                        std::cout << "Punto " << id << " colinda con " << neighborId
                            << (isBidirectional ? " (doble sentido)" : " (una dirección)") << std::endl;
                    }
                    else {
                        std::cerr << "Error en el formato de la conexión: " << connection << std::endl;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error al leer el vecino y la dirección en la conexión: " << connection
                        << " (" << e.what() << ")" << std::endl;
                }

                start = end + 1;
            }
        }
    }
    file.close();
}

void drawGraph(sf::RenderWindow& window, Graph& graph, sf::Font& font) {
    for (const auto& vertex : graph.getVertices()) {
        Node* node = graph.getNode(vertex);
        auto neighbors = graph.getNeighbors(vertex);

        sf::CircleShape shape(10);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(node->getX(), node->getY());
        window.draw(shape);

        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(std::stoi(node->getId().substr(4))));
        text.setCharacterSize(14);
        text.setFillColor(sf::Color::Black);
        text.setPosition(node->getX(), node->getY());
        window.draw(text);

        for (const auto& neighbor : neighbors) {
            string neighborId;
            double weight;
            bool isBidirectional;

            std::tie(neighborId, weight, isBidirectional) = neighbor;
            Node* neighborNode = graph.getNode(neighborId);

            sf::Color lineColor = isBidirectional ? sf::Color::Magenta : sf::Color::Blue;

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(node->getX(), node->getY()), lineColor),
                sf::Vertex(sf::Vector2f(neighborNode->getX(), neighborNode->getY()), lineColor)
            };
            window.draw(line, 2, sf::Lines);
        }
    }
}

void drawShortestPath(sf::RenderWindow& window, Graph& graph, const std::vector<std::string>& path) {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        Node* node = graph.getNode(path[i]);
        Node* nextNode = graph.getNode(path[i + 1]);

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(node->getX(), node->getY()), sf::Color::Green),
            sf::Vertex(sf::Vector2f(nextNode->getX(), nextNode->getY()), sf::Color::Green)
        };
        window.draw(line, 2, sf::Lines);
    }
}

string findNodeAtPosition(Graph& graph, float x, float y, float radius = 20.0f) {
    for (const auto& vertex : graph.getVertices()) {
        Node* node = graph.getNode(vertex);
        float nodeX = node->getX();
        float nodeY = node->getY();

        float distance = sqrt(pow(x - nodeX, 2) + pow(y - nodeY, 2));

        if (distance <= radius) {
            return vertex;
        }
    }
    return "";
}

int main() {
    sf::RenderWindow window;
    sf::Texture texture;
    if (!texture.loadFromFile("Fondo.png")) {
        std::cerr << "Error: No se pudo cargar la imagen desde el archivo." << std::endl;
        return -1;
    }

    sf::Vector2u textureSize = texture.getSize();
    const unsigned int fixedWidth = 1280;
    const unsigned int fixedHeight = 720;
    window.create(sf::VideoMode(fixedWidth, fixedHeight), "SFML Fixed Size Window", sf::Style::Close);

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setScale(
        static_cast<float>(fixedWidth) / textureSize.x,
        static_cast<float>(fixedHeight) / textureSize.y
    );

    Graph graph(true);
    loadVerticesFromFile(graph, "vertices.txt");
    loadGraphFromFile(graph, "puntos.txt");

    Dijkstra dijkstra(&graph);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error: No se pudo cargar la fuente arial.ttf." << endl;
        return -1;
    }

    sf::Texture carTexture;
    if (!carTexture.loadFromFile("carro.png")) {
        cerr << "Error: No se pudo cargar la imagen del carro." << endl;
        return -1;
    }
    sf::Sprite carSprite(carTexture);
    carSprite.setScale(0.1f, 0.1f);

    string startNodeId, endNodeId;
    bool selectingStartNode = true;
    std::vector<std::string> shortestPath;
    size_t pathIndex = 0;
    bool animateCar = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                float mouseX = event.mouseButton.x;
                float mouseY = event.mouseButton.y;
                string clickedNode = findNodeAtPosition(graph, mouseX, mouseY);

                if (!clickedNode.empty()) {
                    if (selectingStartNode) {
                        startNodeId = clickedNode;
                        selectingStartNode = false;
                        cout << "Punto de inicio seleccionado: " << startNodeId << endl;
                    }
                    else {
                        endNodeId = clickedNode;
                        selectingStartNode = true;
                        cout << "Punto de destino seleccionado: " << endNodeId << endl;
                        shortestPath = dijkstra.shortestPath(startNodeId, endNodeId);
                        pathIndex = 0;
                        animateCar = true;

                        if (!shortestPath.empty()) {
                            Node* startNode = graph.getNode(shortestPath[0]);
                            carSprite.setPosition(startNode->getX(), startNode->getY());
                        }
                    }
                }
            }
        }

        window.clear();
        window.draw(sprite);
        drawGraph(window, graph, font);

        if (!shortestPath.empty()) {
            drawShortestPath(window, graph, shortestPath);

            if (animateCar && pathIndex < shortestPath.size() - 1) {
                Node* currentNode = graph.getNode(shortestPath[pathIndex]);
                Node* nextNode = graph.getNode(shortestPath[pathIndex + 1]);

                sf::Vector2f currentPos(currentNode->getX(), currentNode->getY());
                sf::Vector2f nextPos(nextNode->getX(), nextNode->getY());
                sf::Vector2f direction = nextPos - currentPos;

                float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (distance != 0) direction /= distance;

                carSprite.setPosition(carSprite.getPosition() + direction * 2.0f);

                if (sqrt(pow(carSprite.getPosition().x - nextPos.x, 2) + pow(carSprite.getPosition().y - nextPos.y, 2)) < 2.0f) {
                    pathIndex++;
                    carSprite.setPosition(nextPos);
                }
            }

            window.draw(carSprite);
        }

        window.display();
    }

    return 0;
}
