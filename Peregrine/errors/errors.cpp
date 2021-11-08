#include "error.hpp"
#include <iostream>
#include <map>
#include <string>

const std::string prefix = "\e[";
const std::string suffix = "m";
const std::string reset = prefix + "0" + suffix;

const std::string cyan = "36";
const std::string light_red = "91";
const std::string blue = "94";

const std::string bold = "1";

std::string fg(std::string text, std::string color) {
    return prefix + color + suffix + text + reset;
}

std::string style(std::string text, std::string color) {
    return prefix + color + suffix + text + reset;
}

void display(PEError e) {
    std::cout << "  ╭- "
              << fg(style("Error ---------------------------------------- " +
                              e.loc.file + ":" + std::to_string(e.loc.line) +
                              ":" + std::to_string(e.loc.col),
                          bold),
                    light_red)
              << "\n";
    std::cout << "  | " << fg(style(e.msg, bold), light_red) << "\n";
    std::cout << "  |"
              << "\n";
    std::cout << "  |"
              << "\n";
    std::cout << "  |"
              << "\n";
    std::cout << std::to_string(e.loc.line) << " | "
              << e.loc.code.substr(0, e.loc.code.length()) << "\n";
    std::cout << "  |";
    std::cout << std::string(e.loc.col, ' ');
    std::cout << fg("~", light_red);
    std::cout << fg(style(" <----- " + e.submsg, bold), light_red) << "\n";
    std::cout << "  |\n  |\n";
    if (e.hint != "") {
        std::cout << "  ├- " << fg(style("Try: ", bold), blue) << e.hint
                  << "\n";
        std::cout << "  |"
                  << "\n";
    }
    std::cout << "  ╰- " << fg(style("Hint: ", bold), cyan)
              << "Use peregrine --explain=" << e.ecode << "\n";
}