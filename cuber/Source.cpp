#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Dijkstra.h"
#include "Floyd.h"
#include <string>
#include "Incident.h"
#include <deque>
#include <algorithm>
#include <iomanip>

using namespace std;


std::vector<Incident> incidents = {};
const float costPerWeight = 0.01f;
const float costPerStop = 0.01f;

float totalTransportCost = 0.0f;
enum Algorithm { DIJKSTRA, FLOYD_WARSHALL };
std::vector<std::string> originalPath;


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

            // Calcular la posición y la longitud de la línea
            sf::Vector2f start(node->getX(), node->getY());
            sf::Vector2f end(neighborNode->getX(), neighborNode->getY());
            sf::Vector2f direction = end - start;

            float length = sqrt(direction.x * direction.x + direction.y * direction.y);
            float thickness = 1.5f; // Ancho de la línea

            // Crear un rectángulo para la línea con el grosor deseado
            sf::RectangleShape line(sf::Vector2f(length, thickness));
            line.setFillColor(lineColor);
            line.setPosition(start);

            // Rotar el rectángulo para que apunte hacia el nodo vecino
            line.setRotation(atan2(direction.y, direction.x) * 180 / 3.14159265f);

            // Dibujar la línea como un rectángulo
            window.draw(line);

            // Calcular la posición del texto del peso en el medio de la arista
            float midX = (node->getX() + neighborNode->getX()) / 2;
            float midY = (node->getY() + neighborNode->getY()) / 2;

            // Crear y configurar el texto para mostrar el peso con menos decimales
            std::ostringstream weightStream;
            weightStream << std::fixed << std::setprecision(1) << weight; // Una posición decimal

            sf::Text weightText;
            weightText.setFont(font);
            weightText.setString(weightStream.str());
            weightText.setCharacterSize(12);  // Tamaño reducido del texto del peso
            weightText.setFillColor(sf::Color::Black);
            weightText.setPosition(midX, midY);

            // Dibujar el texto del peso
            window.draw(weightText);
        }
    }
}

void adjustTraffic(Graph& graph, FloydWarshall& floydWarshall, const std::string& src, const std::string& dest, int trafficLevel) {
    // Obtén el peso actual de la arista antes de ajustarlo
    double currentWeight = graph.getEdgeWeight(src, dest);
    if (currentWeight <= 0) {
        std::cerr << "Error: no se encontró el peso de la arista entre " << src << " y " << dest << std::endl;
        return;
    }

    // Calcula el nuevo peso multiplicando el peso actual por el nivel de tráfico
    double newWeight = currentWeight * trafficLevel;
    graph.updateEdgeWeight(src, dest, newWeight);

    std::cout << "Nuevo peso para la arista " << src << " -> " << dest << ": " << newWeight << std::endl;

    // Actualiza las matrices de Floyd-Warshall para reflejar el nuevo peso
    floydWarshall.updateMatrices();  // Asegúrate de que `updateMatrices` esté implementado en FloydWarshall
}

void openTrafficWindow(Graph& graph, FloydWarshall& floydWarshall) {
    sf::RenderWindow trafficWindow(sf::VideoMode(350, 250), "Ajustar Tráfico", sf::Style::Titlebar | sf::Style::Close);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf." << std::endl;
        return;
    }

    // Botones de selección de tráfico y configuración de cuadros de texto
    sf::RectangleShape normalTrafficButton(sf::Vector2f(100, 50));
    normalTrafficButton.setFillColor(sf::Color::Green);
    normalTrafficButton.setPosition(200, 60);

    sf::Text normalText("Normal", font, 18);
    normalText.setFillColor(sf::Color::Black);
    normalText.setPosition(215, 70);

    sf::RectangleShape moderateTrafficButton(sf::Vector2f(100, 50));
    moderateTrafficButton.setFillColor(sf::Color::Yellow);
    moderateTrafficButton.setPosition(200, 120);

    sf::Text moderateText("Moderado", font, 18);
    moderateText.setFillColor(sf::Color::Black);
    moderateText.setPosition(205, 130);

    sf::RectangleShape slowTrafficButton(sf::Vector2f(100, 50));
    slowTrafficButton.setFillColor(sf::Color::Red);
    slowTrafficButton.setPosition(200, 180);

    sf::Text slowText("Lento", font, 18);
    slowText.setFillColor(sf::Color::Black);
    slowText.setPosition(215, 190);

    // Etiquetas y cuadros de texto para "Desde" y "Hasta" puntos
    sf::Text fromLabel("Desde:", font, 18);
    fromLabel.setFillColor(sf::Color::Black);
    fromLabel.setPosition(20, 20);

    sf::Text toLabel("Hasta:", font, 18);
    toLabel.setFillColor(sf::Color::Black);
    toLabel.setPosition(20, 80);

    sf::RectangleShape fromBox(sf::Vector2f(100, 25));
    fromBox.setPosition(80, 20);
    fromBox.setFillColor(sf::Color(220, 220, 220));
    fromBox.setOutlineThickness(2);

    sf::RectangleShape toBox(sf::Vector2f(100, 25));
    toBox.setPosition(80, 80);
    toBox.setFillColor(sf::Color(220, 220, 220));
    toBox.setOutlineThickness(2);

    std::string fromStr, toStr;
    sf::Text fromText(fromStr, font, 18);
    fromText.setFillColor(sf::Color::Black);
    fromText.setPosition(85, 22);

    sf::Text toText(toStr, font, 18);
    toText.setFillColor(sf::Color::Black);
    toText.setPosition(85, 82);

    bool isFromSelected = false;
    bool isToSelected = false;

    while (trafficWindow.isOpen()) {
        sf::Event event;
        while (trafficWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                trafficWindow.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (normalTrafficButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && !fromStr.empty() && !toStr.empty()) {
                    adjustTraffic(graph, floydWarshall, "Node" + fromStr, "Node" + toStr, 1);
                    floydWarshall.updateMatrices(); // Actualiza la matriz después de cada ajuste
                    trafficWindow.close();
                }
                else if (moderateTrafficButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && !fromStr.empty() && !toStr.empty()) {
                    adjustTraffic(graph, floydWarshall, "Node" + fromStr, "Node" + toStr, 2);
                    floydWarshall.updateMatrices(); // Actualiza la matriz después de cada ajuste
                    trafficWindow.close();
                }
                else if (slowTrafficButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && !fromStr.empty() && !toStr.empty()) {
                    adjustTraffic(graph, floydWarshall, "Node" + fromStr, "Node" + toStr, 3);
                    floydWarshall.updateMatrices(); // Actualiza la matriz después de cada ajuste
                    trafficWindow.close();
                }
                else if (fromBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isFromSelected = true;
                    isToSelected = false;
                    fromBox.setOutlineColor(sf::Color::Blue);
                    toBox.setOutlineColor(sf::Color::Black);
                }
                else if (toBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isFromSelected = false;
                    isToSelected = true;
                    toBox.setOutlineColor(sf::Color::Blue);
                    fromBox.setOutlineColor(sf::Color::Black);
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char enteredChar = static_cast<char>(event.text.unicode);
                    if (std::isdigit(enteredChar)) {
                        if (isFromSelected && fromStr.length() < 3) {
                            fromStr += enteredChar;
                        }
                        else if (isToSelected && toStr.length() < 3) {
                            toStr += enteredChar;
                        }
                    }
                    else if (event.text.unicode == '\b') {
                        if (isFromSelected && !fromStr.empty()) {
                            fromStr.pop_back();
                        }
                        else if (isToSelected && !toStr.empty()) {
                            toStr.pop_back();
                        }
                    }
                    fromText.setString(fromStr);
                    toText.setString(toStr);
                }
            }
        }

        trafficWindow.clear(sf::Color::White);
        trafficWindow.draw(fromLabel);
        trafficWindow.draw(fromBox);
        trafficWindow.draw(fromText);
        trafficWindow.draw(toLabel);
        trafficWindow.draw(toBox);
        trafficWindow.draw(toText);

        trafficWindow.draw(normalTrafficButton);
        trafficWindow.draw(normalText);
        trafficWindow.draw(moderateTrafficButton);
        trafficWindow.draw(moderateText);
        trafficWindow.draw(slowTrafficButton);
        trafficWindow.draw(slowText);

        trafficWindow.display();
    }
}



// Función para abrir la ventana de incidentes y añadir uno nuevo
void openIncidentWindow(Graph& graph, Algorithm selectedAlgorithm, vector<string>& shortestPath, FloydWarshall& floydWarshall, Dijkstra& dijkstra, string startNodeId, string endNodeId, sf::Text& costText, sf::Sprite& carSprite, bool& animateCar) {

    sf::RenderWindow incidentWindow(sf::VideoMode(400, 400), "Reportar Incidente", sf::Style::Titlebar | sf::Style::Close);

    bool isCoseviSelected = false;
    bool isAccidentSelected = false;
    bool showDirectionOptions = false;
    string selectedDirection = "Ambas Direcciones";
    bool isFromSelected = false;
    bool isToSelected = false;
    string toStr;
    string fromStr;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf." << std::endl;
        return;
    }
    // Configuración de los campos de texto "Desde Punto" y "Hasta Punto"
    sf::Text fromLabel("Desde Punto:", font, 14);
    fromLabel.setPosition(20, 50);
    fromLabel.setFillColor(sf::Color::Black);

    sf::RectangleShape fromBox(sf::Vector2f(100, 25));
    fromBox.setPosition(120, 50);
    fromBox.setFillColor(sf::Color(220, 220, 220));
    fromBox.setOutlineColor(sf::Color::Black);
    fromBox.setOutlineThickness(1);

    sf::Text fromText("", font, 14);
    fromText.setPosition(125, 53);
    fromText.setFillColor(sf::Color::Black);


    sf::Text toLabel("Hasta Punto:", font, 14);
    toLabel.setPosition(20, 90);
    toLabel.setFillColor(sf::Color::Black);

    sf::RectangleShape toBox(sf::Vector2f(100, 25));
    toBox.setPosition(120, 90);
    toBox.setFillColor(sf::Color(220, 220, 220));
    toBox.setOutlineColor(sf::Color::Black);
    toBox.setOutlineThickness(1);

    sf::Text toText("", font, 14);
    toText.setPosition(125, 93);
    toText.setFillColor(sf::Color::Black);


    // Configuración de los botones para tipo de incidente
    sf::Text incidentTypeLabel("Tipo de Incidente:", font, 14);
    incidentTypeLabel.setPosition(20, 150);
    incidentTypeLabel.setFillColor(sf::Color::Black);

    sf::RectangleShape coseviButton(sf::Vector2f(200, 30));
    coseviButton.setPosition(20, 180);
    coseviButton.setFillColor(sf::Color::Blue);

    sf::Text coseviText("Calle cerrada por mantenimiento (COSEVI)", font, 14);
    coseviText.setPosition(25, 185);
    coseviText.setFillColor(sf::Color::White);

    sf::RectangleShape accidentButton(sf::Vector2f(200, 30));
    accidentButton.setPosition(20, 220);
    accidentButton.setFillColor(sf::Color::Green);

    sf::Text accidentText("Accidente", font, 14);
    accidentText.setPosition(25, 225);
    accidentText.setFillColor(sf::Color::White);

    // Botones de dirección
    sf::RectangleShape dir1Button(sf::Vector2f(150, 30));
    dir1Button.setPosition(20, 270);
    dir1Button.setFillColor(sf::Color::Cyan);

    sf::Text dir1Text("Dirección 1", font, 14);
    dir1Text.setPosition(30, 275);
    dir1Text.setFillColor(sf::Color::Black);

    sf::RectangleShape dir2Button(sf::Vector2f(150, 30));
    dir2Button.setPosition(200, 270);
    dir2Button.setFillColor(sf::Color::Cyan);

    sf::Text dir2Text("Dirección 2", font, 14);
    dir2Text.setPosition(210, 275);
    dir2Text.setFillColor(sf::Color::Black);

    // Botón de confirmación
    sf::RectangleShape confirmButton(sf::Vector2f(150, 30));
    confirmButton.setPosition(125, 330);
    confirmButton.setFillColor(sf::Color::Red);

    sf::Text confirmButtonText("Confirmar Incidente", font, 14);
    confirmButtonText.setPosition(130, 335);
    confirmButtonText.setFillColor(sf::Color::White);

    while (incidentWindow.isOpen()) {
        sf::Event event;
        while (incidentWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                incidentWindow.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (coseviButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isCoseviSelected = true;
                    isAccidentSelected = false;
                    // Verificar si ambos puntos son vecinos y si la calle es de doble sentido
                    if (!fromStr.empty() && !toStr.empty() && graph.isDirectNeighbor("Node" + fromStr, "Node" + toStr)) {
                        showDirectionOptions = graph.isDoubleWay("Node" + fromStr, "Node" + toStr);
                    }
                    else {
                        showDirectionOptions = false;
                    }
                    coseviButton.setFillColor(sf::Color::Yellow);
                    accidentButton.setFillColor(sf::Color::Green);
                }
                else if (accidentButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isAccidentSelected = true;
                    isCoseviSelected = false;

                    // Verificar si ambos puntos son vecinos y si la calle es de doble sentido
                    if (!fromStr.empty() && !toStr.empty() && graph.isDirectNeighbor("Node" + fromStr, "Node" + toStr)) {
                        showDirectionOptions = graph.isDoubleWay("Node" + fromStr, "Node" + toStr);
                    }
                    else {
                        showDirectionOptions = false;
                    }

                    accidentButton.setFillColor(sf::Color::Yellow);
                    coseviButton.setFillColor(sf::Color::Blue);
                }

                if (fromBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isFromSelected = true;
                    isToSelected = false;
                    fromBox.setOutlineColor(sf::Color::Blue);
                    toBox.setOutlineColor(sf::Color::Black);
                }
                else if (toBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isFromSelected = false;
                    isToSelected = true;
                    fromBox.setOutlineColor(sf::Color::Black);
                    toBox.setOutlineColor(sf::Color::Blue);
                }
                else {
                    isFromSelected = false;
                    isToSelected = false;
                    fromBox.setOutlineColor(sf::Color::Black);
                    toBox.setOutlineColor(sf::Color::Black);
                }

                if (showDirectionOptions && dir1Button.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    selectedDirection = "Direccion 1";
                    dir1Button.setFillColor(sf::Color::Yellow);
                    dir2Button.setFillColor(sf::Color::Cyan);
                }
                else if (showDirectionOptions && dir2Button.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    selectedDirection = "Direccion 2";
                    dir2Button.setFillColor(sf::Color::Yellow);
                    dir1Button.setFillColor(sf::Color::Cyan);
                }

                if (confirmButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    string fromNodeID = "Node" + fromStr;
                    string toNodeID = "Node" + toStr;

                    if (!graph.isDirectNeighbor(fromNodeID, toNodeID)) {
                        std::cerr << "Error: Los puntos no están conectados directamente." << std::endl;
                        continue;
                    }

                    Incident newIncident;
                    newIncident.fromPoint = fromNodeID;
                    newIncident.toPoint = toNodeID;
                    newIncident.type = isCoseviSelected ? "COSEVI" : "Accidente";
                    newIncident.direction = showDirectionOptions ? selectedDirection : "Ambas Direcciones";
                    incidents.push_back(newIncident);

                    if (selectedAlgorithm == DIJKSTRA) {
                        shortestPath = dijkstra.shortestPath(startNodeId, endNodeId,incidents);
                    }
                    else if (selectedAlgorithm == FLOYD_WARSHALL) {
                        shortestPath = floydWarshall.getShortestPath(startNodeId, endNodeId, incidents);
                    }
                    // Recalcular la ruta después de agregar el incidente
                    if (selectedAlgorithm == DIJKSTRA) {
                        shortestPath = dijkstra.shortestPath(startNodeId, endNodeId, incidents);
                    }
                    else if (selectedAlgorithm == FLOYD_WARSHALL) {
                        shortestPath = floydWarshall.getShortestPath(startNodeId, endNodeId, incidents);
                    }

                    // Iniciar la animación del carro
                    animateCar = true;
                    if (!shortestPath.empty()) {
                        Node* startNode = graph.getNode(shortestPath[0]);
                        carSprite.setPosition(startNode->getX(), startNode->getY());
                    }

                    incidentWindow.close();
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char enteredChar = static_cast<char>(event.text.unicode);
                    if (std::isdigit(enteredChar)) {
                        if (isFromSelected && fromStr.length() < 3) {
                            fromStr += enteredChar;
                        }
                        else if (isToSelected && toStr.length() < 3) {
                            toStr += enteredChar;
                        }
                    }
                    else if (event.text.unicode == '\b') {
                        if (isFromSelected && !fromStr.empty()) {
                            fromStr.pop_back();
                        }
                        else if (isToSelected && !toStr.empty()) {
                            toStr.pop_back();
                        }
                    }
                    fromText.setString(fromStr);
                    toText.setString(toStr);
                }
            }
        }

        incidentWindow.clear(sf::Color::White);
        incidentWindow.draw(fromLabel);
        incidentWindow.draw(fromBox);
        incidentWindow.draw(fromText);
        incidentWindow.draw(toLabel);
        incidentWindow.draw(toBox);
        incidentWindow.draw(toText);
        incidentWindow.draw(incidentTypeLabel);
        incidentWindow.draw(coseviButton);
        incidentWindow.draw(coseviText);
        incidentWindow.draw(accidentButton);
        incidentWindow.draw(accidentText);

        if (showDirectionOptions) {
            incidentWindow.draw(dir1Button);
            incidentWindow.draw(dir1Text);
            incidentWindow.draw(dir2Button);
            incidentWindow.draw(dir2Text);
        }

        incidentWindow.draw(confirmButtonText);
        incidentWindow.draw(confirmButton);

        incidentWindow.display();
    }
}


void drawShortestPath(sf::RenderWindow& window, Graph& graph, const std::vector<std::string>& path, const std::vector<Incident>& incidents, bool isOriginal = false) {
    

    for (size_t i = 0; i < path.size() - 1; ++i) {
        Node* node = graph.getNode(path[i]);
        Node* nextNode = graph.getNode(path[i + 1]);


        // Verificar si hay un incidente en la arista
        bool isAffected = false;
        bool isOppositeDirection = false;
        for (const auto& incident : incidents) {
            if ((incident.fromPoint == path[i] && incident.toPoint == path[i + 1]) ||
                (incident.toPoint == path[i] && incident.fromPoint == path[i + 1])) {
                isAffected = true;
                if (incident.direction == "Direccion 1" && incident.fromPoint == path[i + 1]) {
                    isOppositeDirection = true;
                }
                else if (incident.direction == "Direccion 2" && incident.fromPoint == path[i]) {
                    isOppositeDirection = true;
                }
            }
        }

        // Elegir color y grosor de la línea
        sf::Color lineColor = isOriginal ? sf::Color::Cyan : sf::Color::Green;
        if (isAffected) {
            lineColor = isOppositeDirection ? sf::Color::Yellow : sf::Color::Red;
        }
        float lineThickness = isOriginal ? 3.0f : 1.0f;

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(node->getX(), node->getY()), lineColor),
            sf::Vertex(sf::Vector2f(nextNode->getX(), nextNode->getY()), lineColor)
        };

        // Dibujar la línea con mayor grosor si es la ruta original
        if (isOriginal) {
            for (int j = -1; j <= 1; j++) {
                sf::Vertex thickLine[] = {
                    sf::Vertex(sf::Vector2f(node->getX() + j, node->getY() + j), lineColor),
                    sf::Vertex(sf::Vector2f(nextNode->getX() + j, nextNode->getY() + j), lineColor)
                };
                window.draw(thickLine, 2, sf::Lines);
            }
        }
        else {
            window.draw(line, 2, sf::Lines);
        }
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

float calculateTransportCost(const std::vector<std::string>& path, Graph& graph, float costPerWeight, float costPerStop, float elapsedTime) {
    float totalCost = 0.0f;
    float totalWeight = 0.0f; // Acumula el peso total del camino

    std::cout << "Desglose detallado del cálculo de transporte:\n";
    std::cout << "--------------------------------------------\n";

    for (size_t i = 0; i < path.size() - 1; ++i) {
        std::string currentNode = path[i];
        std::string nextNode = path[i + 1];

        // Obtén el peso directamente desde el grafo
        float edgeWeight = graph.getEdgeWeight(currentNode, nextNode);
        if (edgeWeight < 0) {
            std::cerr << "Error: No se encontró una arista válida entre " << currentNode << " y " << nextNode << "\n";
            continue;
        }

        float weightCost = edgeWeight * costPerWeight;
        float stopCost = costPerStop;

        totalWeight += edgeWeight; // Suma el peso actual al peso total
        totalCost += weightCost + stopCost;

        // Mensajes de depuración detallados
        std::cout << "Segmento: " << currentNode << " -> " << nextNode << "\n";
        std::cout << " - Peso de la arista: " << edgeWeight << "\n";
        std::cout << " - Costo por peso en este segmento: $" << weightCost << "\n";
        std::cout << " - Costo de detención en este segmento: $" << stopCost << "\n";
        std::cout << " - Costo acumulado hasta ahora: $" << totalCost << "\n\n";
    }

    // Agregar el costo de detención para el último nodo
    totalCost += costPerStop;
    std::cout << "Costo de detención final: $" << costPerStop << "\n";

    // Sumar el costo basado en el tiempo transcurrido
    float timeCost = elapsedTime;
    totalCost += timeCost;

    // Mensajes finales de depuración
    std::cout << "--------------------------------------------\n";
    std::cout << "Tiempo total transcurrido: " << elapsedTime << " s\n";
    std::cout << "Peso total del recorrido: " << totalWeight << "\n";
    std::cout << "Costo total de transporte: $" << totalCost << "\n";
    std::cout << "--------------------------------------------\n";

    return totalCost;
}

// Declaración de la función de reinicio
void resetApplication(Graph& graph, sf::Text& costText, std::vector<std::string>& shortestPath,
    std::vector<std::string>& originalPath, size_t& pathIndex, bool& animateCar,
    bool& timerRunning, sf::Clock& clock, float& totalElapsedTime, sf::Sprite& carSprite,
    std::string& startNodeId, std::string& endNodeId, bool& selectingStartNode,
    Algorithm& selectedAlgorithm, bool buttonPressed, sf::Text& timerText) { // Cambiar a referencia

    // Reiniciar variables de selección y costos
    
    if (buttonPressed) {
        costText.setString("Costo total: $0.0");
        timerText.setString("Tiempo: 0.0 s");  // Reiniciar el texto del cronómetro
        shortestPath.clear();
        originalPath.clear();
    }

    pathIndex = 0;
    animateCar = false;
    timerRunning = false;
    totalElapsedTime = 0.0f;

    // Reiniciar el cronómetro a cero
    clock.restart();

    // Reiniciar posiciones de nodos seleccionados y variables de control
    startNodeId = "";
    endNodeId = "";
    selectingStartNode = true;

    // Reposicionar el carro fuera de vista o en la posición inicial
    carSprite.setPosition(-100, -100); // Fuera de la vista principal

    // Reiniciar algoritmo seleccionado (ninguno por defecto)
    selectedAlgorithm = DIJKSTRA;
    // Reiniciar los incidentes puestos anteriormente
    incidents = {};
}




int main()
{
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
    bool originalPathSet = false;
    sf::Time elapsedTime;
    float totalElapsedTime;

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
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf." << std::endl;
        return -1;
    }

    sf::Texture carTexture;
    if (!carTexture.loadFromFile("carro.png")) {
        std::cerr << "Error: No se pudo cargar la imagen del carro." << std::endl;
        return -1;
    }
    sf::Sprite carSprite(carTexture);
    carSprite.setScale(0.1f, 0.1f);

    string startNodeId, endNodeId;
    bool selectingStartNode = true;
    vector<string> shortestPath;
    size_t pathIndex = 0;
    bool animateCar = false;

    Algorithm selectedAlgorithm = DIJKSTRA;

    // Texto y fondo para el costo
    sf::Text costText("Costo total: $0.0", font, 20);
    costText.setFillColor(sf::Color::Black);
    costText.setPosition(500, fixedHeight - 50);

    sf::RectangleShape costTextBackground(sf::Vector2f(200, 30));
    costTextBackground.setFillColor(sf::Color::White);
    costTextBackground.setPosition(500, fixedHeight - 50);

    // Botones de algoritmos
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
   

    // Cronómetro y texto del cronómetro
    sf::Clock clock;
    sf::Text timerText("", font, 20);
    timerText.setFillColor(sf::Color::Black);
    timerText.setPosition(10, 10);
    bool timerRunning = false;

    float costPerWeight = 0.5f; // Valor por unidad de distancia
    float costPerStop = 1.0f;   // Valor por cada nodo de detención

    // Crear botón de "Incidentes"
    sf::RectangleShape menuButton(sf::Vector2f(100, 50));
    menuButton.setFillColor(sf::Color::Green);
    menuButton.setPosition(fixedWidth - 110, 10);

    sf::Text buttonText("Incidentes", font, 20);
    buttonText.setFillColor(sf::Color::Black);
    sf::FloatRect textRect = buttonText.getLocalBounds();
    buttonText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    buttonText.setPosition(
        menuButton.getPosition().x + menuButton.getSize().x / 2.0f,
        menuButton.getPosition().y + menuButton.getSize().y / 2.0f
    );

    // Crear botón de "Tráfico"
    sf::RectangleShape trafficButton(sf::Vector2f(100, 50));
    trafficButton.setFillColor(sf::Color::Yellow);
    trafficButton.setPosition(fixedWidth - 110, 70);

    sf::Text trafficButtonText("Tráfico", font, 20);
    trafficButtonText.setFillColor(sf::Color::Black);
    sf::FloatRect trafficTextRect = trafficButtonText.getLocalBounds();
    trafficButtonText.setOrigin(trafficTextRect.left + trafficTextRect.width / 2.0f, trafficTextRect.top + trafficTextRect.height / 2.0f);
    trafficButtonText.setPosition(
        trafficButton.getPosition().x + trafficButton.getSize().x / 2.0f,
        trafficButton.getPosition().y + trafficButton.getSize().y / 2.0f
    );

    sf::RectangleShape resetButton(sf::Vector2f(100, 50));
    resetButton.setFillColor(sf::Color::Red);
    resetButton.setPosition(fixedWidth - 110, 130);

    sf::Text resetButtonText("Reiniciar", font, 20);
    resetButtonText.setFillColor(sf::Color::Black);
    sf::FloatRect resetTextRect = resetButtonText.getLocalBounds();
    resetButtonText.setOrigin(resetTextRect.left + resetTextRect.width / 2.0f, resetTextRect.top + resetTextRect.height / 2.0f);
    resetButtonText.setPosition(
        resetButton.getPosition().x + resetButton.getSize().x / 2.0f,
        resetButton.getPosition().y + resetButton.getSize().y / 2.0f
    );

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                float mouseX = event.mouseButton.x;
                float mouseY = event.mouseButton.y;

                if (menuButton.getGlobalBounds().contains(mouseX, mouseY))
                {
                    openIncidentWindow(graph, selectedAlgorithm, shortestPath, floydWarshall, dijkstra, startNodeId, endNodeId, costText, carSprite, animateCar);
                }
                if (trafficButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    openTrafficWindow(graph, floydWarshall);
                    floydWarshall.updateMatrices();

                    if (!startNodeId.empty() && !endNodeId.empty()) {
                        if (selectedAlgorithm == DIJKSTRA) {
                            shortestPath = dijkstra.shortestPath(startNodeId, endNodeId, incidents);
                        } else if (selectedAlgorithm == FLOYD_WARSHALL) {
                            shortestPath = floydWarshall.getShortestPath(startNodeId, endNodeId, incidents);
                        }
                        std::cout << "Ruta recalculada después de ajuste de tráfico." << std::endl;
                    }
                }
                if (dijkstraButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    selectedAlgorithm = DIJKSTRA;
                    std::cout << "Algoritmo Dijkstra seleccionado" << std::endl;
                }
                else if (floydButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    selectedAlgorithm = FLOYD_WARSHALL;
                    std::cout << "Algoritmo Floyd-Warshall seleccionado" << std::endl;
                }
                else if (startButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    if (!startNodeId.empty() && !endNodeId.empty()) {
                        if (selectedAlgorithm == DIJKSTRA) {
                            shortestPath = dijkstra.shortestPath(startNodeId, endNodeId, incidents);
                        } else if (selectedAlgorithm == FLOYD_WARSHALL) {
                            shortestPath = floydWarshall.getShortestPath(startNodeId, endNodeId, incidents);
                        }
                        pathIndex = 0;
                        animateCar = true;
                        originalPath = shortestPath;

                        if (!shortestPath.empty()) {
                            Node* startNode = graph.getNode(shortestPath[0]);
                            carSprite.setPosition(startNode->getX(), startNode->getY());
                        }

                        // Inicia el cronómetro
                        clock.restart();
                        timerRunning = true;
                    }
                }
                // Verificar si se presionó el botón de reinicio
                else if (resetButton.getGlobalBounds().contains(mouseX, mouseY)) {
                    resetApplication(graph, costText, shortestPath, originalPath, pathIndex, animateCar, timerRunning, clock, totalElapsedTime, carSprite, startNodeId, endNodeId, selectingStartNode, selectedAlgorithm, true, timerText);
                }
                else {
                    string clickedNode = findNodeAtPosition(graph, mouseX, mouseY);
                    if (!clickedNode.empty()) {
                        if (selectingStartNode) {
                            startNodeId = clickedNode;
                            selectingStartNode = false;
                            std::cout << "Punto de inicio seleccionado: " << startNodeId << std::endl;
                        } else {
                            endNodeId = clickedNode;
                            selectingStartNode = true;
                            std::cout << "Punto de destino seleccionado: " << endNodeId << std::endl;
                        }
                    }
                }
            }
        }

        if (timerRunning) {
            sf::Time elapsedTime = clock.getElapsedTime();
            std::ostringstream timerStream;
            timerStream << "Tiempo: " << std::fixed << std::setprecision(2) << elapsedTime.asSeconds() << " s";
            timerText.setString(timerStream.str());
        }

        window.clear();
        window.draw(sprite);

        // Dibujar el grafo
        drawGraph(window, graph, font);

        if (!originalPath.empty()) {
            drawShortestPath(window, graph, originalPath, incidents, true);
        }

        if (!shortestPath.empty()) {
            drawShortestPath(window, graph, shortestPath, incidents);
            if (animateCar && pathIndex < shortestPath.size() - 1) {
                Node* currentNode = graph.getNode(shortestPath[pathIndex]);
                Node* nextNode = graph.getNode(shortestPath[pathIndex + 1]);

                sf::Vector2f currentPos(currentNode->getX(), currentNode->getY());
                sf::Vector2f nextPos(nextNode->getX(), nextNode->getY());
                sf::Vector2f direction = nextPos - currentPos;

                float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (distance != 0) direction /= distance;

                carSprite.move(direction * 2.0f);

                if (sqrt(pow(carSprite.getPosition().x - nextPos.x, 2) + pow(carSprite.getPosition().y - nextPos.y, 2)) < 2.0f) {
                    pathIndex++;
                    carSprite.setPosition(nextPos);

                    if (pathIndex == shortestPath.size() - 1) {
                        animateCar = false;
                        timerRunning = false;

                        elapsedTime = clock.getElapsedTime();
                        totalElapsedTime = elapsedTime.asSeconds();

                        float totalTransportCost = calculateTransportCost(shortestPath, graph, costPerWeight, costPerStop, totalElapsedTime);
                        std::ostringstream costStream;
                        costStream << std::fixed << std::setprecision(4) << totalTransportCost;
                        costText.setString("Costo total: $" + costStream.str());

                        // Llamar a resetApplication después de llegar al destino
                        resetApplication(graph, costText, shortestPath, originalPath, pathIndex, animateCar, timerRunning, clock, totalElapsedTime, carSprite, startNodeId, endNodeId, selectingStartNode, selectedAlgorithm, false, timerText);
                    }
                }
            }
            window.draw(carSprite);
        }

        window.draw(menuButton);
        window.draw(buttonText);
        window.draw(trafficButton);
        window.draw(trafficButtonText);
        window.draw(dijkstraButton);
        window.draw(floydButton);
        window.draw(startButton);
        window.draw(dijkstraText);
        window.draw(floydText);
        window.draw(startText);
        window.draw(costTextBackground);
        window.draw(costText);
        window.draw(timerText);

        // Dibujar el botón de reinicio
        window.draw(resetButton);
        window.draw(resetButtonText);

        window.display();
    }

    return 0;
}
