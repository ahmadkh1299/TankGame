#pragma once

#include <string>

enum class ActionRequest { //notice that "MoveBackward" is not actually moving, but asking to move back
    MoveForward, MoveBackward,
    RotateLeft90, RotateRight90, RotateLeft45, RotateRight45,
    Shoot, GetBattleInfo, DoNothing
};

// namespace to keep things organized and prevents global pollution
namespace ActionUtils {
    inline std::string toString(ActionRequest action) {
        switch (action) {
        case ActionRequest::MoveForward:       return "Move Forward";
        case ActionRequest::MoveBackward:      return "Move Backward";
        case ActionRequest::RotateLeft90:      return "Rotate Left 90";
        case ActionRequest::RotateRight90:     return "Rotate Right 90";
        case ActionRequest::RotateLeft45:      return "Rotate Left 45";
        case ActionRequest::RotateRight45:     return "Rotate Right 45";
        case ActionRequest::Shoot:             return "Shoot";
        case ActionRequest::GetBattleInfo:     return "Get Battle Info";
        case ActionRequest::DoNothing:         return "Do Nothing";
        default:                               return "Unknown";
        }
    }
}
