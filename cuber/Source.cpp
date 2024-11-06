#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Dijkstra.h"
#include "Floyd.h"
#include <string>
#include <deque>
#include <algorithm>

using namespace std;
const float costPerWeight = 2.0f;   // Costo por unidad de peso/distancia
const float costPerStop = 1.0f;     // Costo por tiempo de detención en cada nodo
float totalTransportCost = 0.0f;    // Variable para almacenar el costo total de transporte


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

            std::getline(file, line); // Leer la línea de "Vecinos"

            std::getline(file, line); // Leer la línea de "Calle Doble Sentido"
            size_t start = line.find(":") + 1;

            // Procesar cada conexión en la línea "Calle Doble Sentido"
            while ((start = line.find('(', start)) != std::string::npos) {
                size_t end = line.find(')', start);
                if (end == std::string::npos) break;

                std::string connection = line.substr(start + 1, end - start - 1);

                int neighborId;
                std::string direction;
                float weight = 1.0; // Valor por defecto en caso de error

                try {
                    size_t commaPos = connection.find(',');
                    size_t secondCommaPos = connection.find_last_of(',');

                    if (commaPos != std::string::npos && secondCommaPos != commaPos) {
                        // Leer el ID del vecino
                        neighborId = std::stoi(connection.substr(0, commaPos));

                        // Leer el valor de dirección (True/False)
                        direction = connection.substr(commaPos + 1, secondCommaPos - commaPos - 1);
                        direction.erase(std::remove(direction.begin(), direction.end(), ' '), direction.end());
                        bool isBidirectional = (direction == "True");

                        // Leer el peso
                        weight = std::stof(connection.substr(secondCommaPos + 1));

                        std::string neighborName = "Node" + std::to_string(neighborId);

                        // Agregar la arista al grafo con el peso
                        graph.addEdge(nodeName, neighborName, weight, isBidirectional);

                        std::cout << "Punto " << id << " colinda con " << neighborId
                            << (isBidirectional ? " (doble sentido)" : " (una dirección)")
                            << " con peso " << weight << std::endl;
                    }
                    else {
                        std::cerr << "Error en el formato de la conexión: " << connection << std::endl;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error al leer el vecino, dirección o peso en la conexión: " << connection
                        << " (" << e.what() << ")" << std::endl;
                }

                start = end + 1; // Mover al siguiente elemento en la línea
            }
        }
    }
    file.close();
}

#include <iomanip>
#include <sstream>

void drawGraph(sf::RenderWindow& window, Graph& graph, sf::Font& font) {
    for (const auto& vertex : graph.getVertices()) {
        Node* node = graph.getNode(vertex);
        auto neighbors = graph.getNeighbors(vertex);

        // Dibujar el nodo
        sf::CircleShape shape(10);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(node->getX(), node->getY());
        window.draw(shape);

        // Dibujar el ID del nodo
        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(std::stoi(node->getId().substr(4))));
        text.setCharacterSize(12);  // Tamaño reducido del ID del nodo
        text.setFillColor(sf::Color::Black);
        text.setPosition(node->getX(), node->getY());
        window.draw(text);

        // Dibujar las conexiones (aristas) entre el nodo y sus vecinos
        for (const auto& neighbor : neighbors) {
            string neighborId;
            double weight;
            bool isBidirectional;

            std::tie(neighborId, weight, isBidirectional) = neighbor;
            Node* neighborNode = graph.getNode(neighborId);

            // Determinar el color de la arista
            sf::Color lineColor = isBidirectional ? sf::Color::Magenta : sf::Color::Blue;

            // Crear y dibujar la línea entre el nodo y su vecino
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(node->getX(), node->getY()), lineColor),
                sf::Vertex(sf::Vector2f(neighborNode->getX(), neighborNode->getY()), lineColor)
            };
            window.draw(line, 2, sf::Lines);

            // Calcular la posición del texto del peso en el medio de la arista
            float midX = (node->getX() + neighborNode->getX()) / 2;
            float midY = (node->getY() + neighborNode->getY()) / 2;

            // Crear y configurar el texto para mostrar el peso con menos decimales
            std::ostringstream weightStream;
            weightStream << std::fixed << std::setprecision(1) << weight; // Una posición decimal

            sf::Text weightText;
            weightText.setFont(font);
            weightText.setString(weightStream.str());
            weightText.setCharacterSize(10);  // Tamaño reducido del texto del peso
            weightText.setFillColor(sf::Color::Black);
            weightText.setPosition(midX, midY);

            // Dibujar el texto del peso
            window.draw(weightText);
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
    FloydWarshall floydWarshall(&graph);  
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

    // Variables para selección de algoritmo
    enum Algorithm { DIJKSTRA, FLOYD_WARSHALL };
    Algorithm selectedAlgorithm = DIJKSTRA;

    FloydWarshall floyd(&graph);

    sf::Text costText("Costo total: $0.0", font, 20);
    costText.setFillColor(sf::Color::Black);
    costText.setPosition(500, fixedHeight - 50);

    // Botones para seleccionar el algoritmo
    sf::RectangleShape dijkstraButton(sf::Vector2f(150, 50));
    dijkstraButton.setFillColor(sf::Color::Green);
    dijkstraButton.setPosition(10, fixedHeight - 60);

    sf::RectangleShape floydButton(sf::Vector2f(150, 50));
    floydButton.setFillColor(sf::Color::Blue);
    floydButton.setPosition(170, fixedHeight - 60);

    sf::RectangleShape startButton(sf::Vector2f(150, 50));
    startButton.setFillColor(sf::Color::Yellow);
    startButton.setPosition(330, fixedHeight - 60);

    sf::Text dijkstraText("Dijkstra", font, 20);
    dijkstraText.setFillColor(sf::Color::Black);
    dijkstraText.setPosition(25, fixedHeight - 50);

    sf::Text floydText("Floyd-Warshall", font, 20);
    floydText.setFillColor(sf::Color::Black);
    floydText.setPosition(180, fixedHeight - 50);

    sf::Text startText("Iniciar", font, 20);
    startText.setFillColor(sf::Color::Black);
    startText.setPosition(350, fixedHeight - 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                float mouseX = event.mouseButton.x;
                float mouseY = event.mouseButton.y;

                // Detectar si se selecciona el botón de Dijkstra
                if (dijkstraButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    selectedAlgorithm = DIJKSTRA;
                    std::cout << "Algoritmo Dijkstra seleccionado" << std::endl;
                }
                // Detectar si se selecciona el botón de Floyd-Warshall
                else if (floydButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    selectedAlgorithm = FLOYD_WARSHALL;
                    std::cout << "Algoritmo Floyd-Warshall seleccionado" << std::endl;
                }
                // Detectar si se selecciona el botón de Iniciar
                else if (startButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    if (!startNodeId.empty() && !endNodeId.empty()) {
                        if (selectedAlgorithm == DIJKSTRA) {
                            shortestPath = dijkstra.shortestPath(startNodeId, endNodeId);
                        }
                        else if (selectedAlgorithm == FLOYD_WARSHALL) {
                            shortestPath = floydWarshall.getShortestPath(startNodeId, endNodeId);
                        }
                        pathIndex = 0;
                        animateCar = true;

                        // Inicializar la posición del carro al primer nodo en la ruta más corta
                        if (!shortestPath.empty()) {
                            Node* startNode = graph.getNode(shortestPath[0]);
                            carSprite.setPosition(startNode->getX(), startNode->getY());
                        }


                        totalTransportCost = 0.0f;
                        for (size_t i = 0; i < shortestPath.size() - 1; ++i) {
                            Node* currentNode = graph.getNode(shortestPath[i]);
                            Node* nextNode = graph.getNode(shortestPath[i + 1]);

                            sf::Vector2f currentPos(currentNode->getX(), currentNode->getY());
                            sf::Vector2f nextPos(nextNode->getX(), nextNode->getY());

                            float distance = sqrt(pow(nextPos.x - currentPos.x, 2) + pow(nextPos.y - currentPos.y, 2));
                            float weightCost = distance * costPerWeight;
                            totalTransportCost += weightCost + costPerStop;
                        }

                        costText.setString("Costo total: $" + std::to_string(totalTransportCost));
                    }
                }
                // Selección de nodos de inicio y fin
                else {
                    string clickedNode = findNodeAtPosition(graph, mouseX, mouseY);

                    if (!clickedNode.empty()) {
                        if (selectingStartNode) {
                            startNodeId = clickedNode;
                            selectingStartNode = false;
                            std::cout << "Punto de inicio seleccionado: " << startNodeId << std::endl;
                        }
                        else {
                            endNodeId = clickedNode;
                            selectingStartNode = true;
                            std::cout << "Punto de destino seleccionado: " << endNodeId << std::endl;
                        }
                    }
                }
            }
        }

        window.clear();
        window.draw(sprite); // Fondo

        // Dibujar el grafo
        drawGraph(window, graph, font);

        // Dibujar la ruta más corta si está disponible
        if (!shortestPath.empty()) {
            drawShortestPath(window, graph, shortestPath);

            // Animación del carro a lo largo de la ruta más corta
            if (animateCar && pathIndex < shortestPath.size() - 1) {
                Node* currentNode = graph.getNode(shortestPath[pathIndex]);
                Node* nextNode = graph.getNode(shortestPath[pathIndex + 1]);

                // Posición actual y la posición del próximo nodo
                sf::Vector2f currentPos(currentNode->getX(), currentNode->getY());
                sf::Vector2f nextPos(nextNode->getX(), nextNode->getY());
                sf::Vector2f direction = nextPos - currentPos;

                // Normalizar dirección
                float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (distance != 0) direction /= distance;

                // Mover el carro hacia el siguiente nodo en la ruta
                carSprite.move(direction * 2.0f); // Velocidad de movimiento

                // Revisar si el carro ha llegado al nodo siguiente
                if (sqrt(pow(carSprite.getPosition().x - nextPos.x, 2) + pow(carSprite.getPosition().y - nextPos.y, 2)) < 2.0f) {
                    pathIndex++;
                    carSprite.setPosition(nextPos); // Alinear exactamente al nodo siguiente
                }
            }

            // Dibujar el carro en su posición actual
            window.draw(carSprite);
        }

        // Dibujar botones y etiquetas de texto
        window.draw(dijkstraButton);
        window.draw(floydButton);
        window.draw(startButton);
        window.draw(dijkstraText);
        window.draw(floydText);
        window.draw(startText);
        window.draw(costText);

        window.display();
    }


    return 0;
}
