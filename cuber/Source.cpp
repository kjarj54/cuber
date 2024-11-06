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
#include <iomanip>
#include "Incident.cpp"


using namespace std;
const float costPerWeight = 0.01f;
const float costPerStop = 0.01f;
float totalTransportCost = 0.0f;    


std::vector<Incident> incidents;


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
            weightText.setCharacterSize(12);  // Tamaño reducido del texto del peso
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

// Función para abrir la ventana de incidentes y añadir uno nuevo
void openIncidentWindow(Graph& graph) {
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

// Pantalla para seleccionar puntos A y B, algoritmo y listar incidentes
void openMenuWindow(Graph& graph) {
    sf::RenderWindow menuWindow(sf::VideoMode(500, 400), "Menu", sf::Style::Titlebar | sf::Style::Close);

    //variables
    string inputStrA;
    string inputStrB;
    bool isInputASelected = false;
    bool isInputBSelected = false;


    // Cargar la fuente
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf." << std::endl;
        return;
    }

    // Texto para Punto A y Punto B
    sf::Text labelA("Punto A:", font, 14);
    labelA.setPosition(20, 20);
    labelA.setFillColor(sf::Color::Black);

    sf::Text labelB("Punto B:", font, 14);
    labelB.setPosition(20, 60);
    labelB.setFillColor(sf::Color::Black);

    // Fondo de los campos de entrada
    sf::RectangleShape inputBoxA(sf::Vector2f(100, 25));
    inputBoxA.setPosition(100, 20);
    inputBoxA.setFillColor(sf::Color(220, 220, 220));  // Gris claro
    inputBoxA.setOutlineColor(sf::Color::Black);
    inputBoxA.setOutlineThickness(1);

    sf::RectangleShape inputBoxB(sf::Vector2f(100, 25));
    inputBoxB.setPosition(100, 60);
    inputBoxB.setFillColor(sf::Color(220, 220, 220));  // Gris claro
    inputBoxB.setOutlineColor(sf::Color::Black);
    inputBoxB.setOutlineThickness(1);

    // Campos de texto para mostrar los valores de entrada
    sf::Text inputA("", font, 14);
    inputA.setPosition(105, 23);  // Posición ajustada para centrar el texto dentro del cuadro
    inputA.setFillColor(sf::Color::Black);
    

    sf::Text inputB("", font, 14);
    inputB.setPosition(105, 63);  // Posición ajustada para centrar el texto dentro del cuadro
    inputB.setFillColor(sf::Color::Black);

    // Botones de selección de algoritmo
    sf::RectangleShape floydButton(sf::Vector2f(120, 30));
    floydButton.setPosition(20, 100);
    floydButton.setFillColor(sf::Color::Blue);

    sf::Text floydText("Floyd", font, 14);
    floydText.setPosition(30, 105);
    floydText.setFillColor(sf::Color::Black);

    sf::RectangleShape dijkstraButton(sf::Vector2f(120, 30));
    dijkstraButton.setPosition(160, 100);
    dijkstraButton.setFillColor(sf::Color::Green);

    sf::Text dijkstraText("Dijkstra", font, 14);
    dijkstraText.setPosition(170, 105);
    dijkstraText.setFillColor(sf::Color::Black);

    bool useFloyd = false;

    // Botón para crear lista de incidentes
    sf::RectangleShape incidentButton(sf::Vector2f(200, 40));
    incidentButton.setPosition(100, 200);
    incidentButton.setFillColor(sf::Color::Red);

    sf::Text incidentButtonText("Listar Calles con Incidentes", font, 14);
    incidentButtonText.setPosition(105, 210);
    incidentButtonText.setFillColor(sf::Color::White);

    sf::Text incidentListLabel("Lista de Incidentes:", font, 14);
    incidentListLabel.setPosition(20, 150);
    incidentListLabel.setFillColor(sf::Color::Black);

    while (menuWindow.isOpen()) {
        sf::Event event;
        while (menuWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                menuWindow.close();

            // Detección de selección de algoritmo
            if (event.type == sf::Event::MouseButtonPressed) {
                if (floydButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    useFloyd = true;
                    floydButton.setFillColor(sf::Color::Yellow);
                    dijkstraButton.setFillColor(sf::Color::Green);
                }
                else if (dijkstraButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    useFloyd = false;
                    dijkstraButton.setFillColor(sf::Color::Yellow);
                    floydButton.setFillColor(sf::Color::Blue);
                }

                if (incidentButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    openIncidentWindow(graph);
                }

                // Selección de campo de texto
                if (inputBoxA.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isInputASelected = true;
                    isInputBSelected = false;
                    inputBoxA.setOutlineColor(sf::Color::Blue);
                    inputBoxB.setOutlineColor(sf::Color::Black);
                }
                else if (inputBoxB.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isInputASelected = false;
                    isInputBSelected = true;
                    inputBoxA.setOutlineColor(sf::Color::Black);
                    inputBoxB.setOutlineColor(sf::Color::Blue);
                }
                else {
                    // Desactivar ambos campos si se hace clic fuera de ellos
                    isInputASelected = false;
                    isInputBSelected = false;
                    inputBoxA.setOutlineColor(sf::Color::Black);
                    inputBoxB.setOutlineColor(sf::Color::Black);
                }
            }

            // Captura de texto para los puntos A y B
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char enteredChar = static_cast<char>(event.text.unicode);
                    if (std::isdigit(enteredChar)) {
                        if (isInputASelected && inputStrA.length() < 3) {
                            inputStrA += enteredChar;  // Añadir carácter al campo A
                        }
                        else if (isInputBSelected && inputStrB.length() < 3) {
                            inputStrB += enteredChar;  // Añadir carácter al campo B
                        }
                    }
                    else if (event.text.unicode == '\b') {  // Detectar retroceso
                        if (isInputASelected && !inputStrA.empty()) {
                            inputStrA.pop_back();  // Borrar último carácter de A
                        }
                        else if (isInputBSelected && !inputStrB.empty()) {
                            inputStrB.pop_back();  // Borrar último carácter de B
                        }
                    }
                    inputA.setString(inputStrA);
                    inputB.setString(inputStrB);
                }
            }

        }
    }
    return "";
}


float calculateTransportCost(const std::vector<std::string>& path, Graph& graph, float costPerWeight, float costPerStop) {
    float totalCost = 0.0f;

        menuWindow.clear(sf::Color::White);  // Fondo blanco

        // Dibujar etiquetas y cajas de entrada
        menuWindow.draw(labelA);
        menuWindow.draw(inputBoxA);
        menuWindow.draw(inputA);

        menuWindow.draw(labelB);
        menuWindow.draw(inputBoxB);
        menuWindow.draw(inputB);

        // Dibujar botones de selección de algoritmo y sus textos
        menuWindow.draw(floydButton);
        menuWindow.draw(floydText);
        menuWindow.draw(dijkstraButton);
        menuWindow.draw(dijkstraText);

        // Dibujar botón de incidentes y su texto
        menuWindow.draw(incidentButton);
        menuWindow.draw(incidentButtonText);

        menuWindow.draw(incidentListLabel);

        // Mostrar incidentes
        float yPosition = 180;
        for (size_t i = 0; i < incidents.size(); ++i) {
            sf::Text incidentText("Incidente " + std::to_string(i + 1) + ": " + incidents[i].type +
                " desde " + incidents[i].fromPoint + " hasta " + incidents[i].toPoint,
                font, 12);
            incidentText.setPosition(20, yPosition);
            incidentText.setFillColor(sf::Color::Black);
            menuWindow.draw(incidentText);

            yPosition += 20;
        }

    for (size_t i = 0; i < path.size() - 1; ++i) {
        Node* currentNode = graph.getNode(path[i]);
        Node* nextNode = graph.getNode(path[i + 1]);

        sf::Vector2f currentPos(currentNode->getX(), currentNode->getY());
        sf::Vector2f nextPos(nextNode->getX(), nextNode->getY());

        // Calcular la distancia/peso entre los nodos
        float distance = sqrt(pow(nextPos.x - currentPos.x, 2) + pow(nextPos.y - currentPos.y, 2)) / 100.0f;
        float weightCost = distance * costPerWeight;

        // Agregar el costo de detención para el nodo actual
        float stopCost = costPerStop;

        // Sumar los costos de peso y detención al costo total
        totalCost += weightCost + stopCost;
    }

    // Agregar el costo de detención para el último nodo (si es necesario)
    totalCost += costPerStop;

    return totalCost;
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
    std::vector<std::string> shortestPath;
    size_t pathIndex = 0;
    bool animateCar = false;

    enum Algorithm { DIJKSTRA, FLOYD_WARSHALL };
    Algorithm selectedAlgorithm = DIJKSTRA;

    sf::Text costText("Costo total: $0.0", font, 20);
    costText.setFillColor(sf::Color::Black);
    costText.setPosition(500, fixedHeight - 50);

    sf::RectangleShape costTextBackground(sf::Vector2f(200, 30)); 
    costTextBackground.setFillColor(sf::Color::White);            
    costTextBackground.setPosition(500, fixedHeight - 50);       


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

    float costPerWeight = 0.5f; // Valor por unidad de distancia
    float costPerStop = 1.0f;   // Valor por cada nodo de detención

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                float mouseX = event.mouseButton.x;
                float mouseY = event.mouseButton.y;

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
                            shortestPath = dijkstra.shortestPath(startNodeId, endNodeId);
                        }
                        else if (selectedAlgorithm == FLOYD_WARSHALL) {
                            shortestPath = floydWarshall.getShortestPath(startNodeId, endNodeId);
                        }
                        pathIndex = 0;
                        animateCar = true;

                        if (!shortestPath.empty()) {
                            Node* startNode = graph.getNode(shortestPath[0]);
                            carSprite.setPosition(startNode->getX(), startNode->getY());
                        }

                        float totalTransportCost = calculateTransportCost(shortestPath, graph, costPerWeight, costPerStop);
                        std::ostringstream costStream;
                        costStream << std::fixed << std::setprecision(4) << totalTransportCost;

                        // Asignar el costo al texto con el símbolo de colón
                        costText.setString("Costo total: $" + costStream.str());
                    }
                }
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

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (menuButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    openMenuWindow(graph);

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

                carSprite.move(direction * 2.0f);

                if (sqrt(pow(carSprite.getPosition().x - nextPos.x, 2) + pow(carSprite.getPosition().y - nextPos.y, 2)) < 2.0f) {
                    pathIndex++;
                    carSprite.setPosition(nextPos);
                }
            }

            window.draw(carSprite);
        }

        window.draw(dijkstraButton);
        window.draw(floydButton);
        window.draw(startButton);
        window.draw(dijkstraText);
        window.draw(floydText);
        window.draw(startText);
        window.draw(costTextBackground);
        window.draw(costText);


        window.display();
    }

    return 0;
}
