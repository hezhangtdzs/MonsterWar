#include "observer.h"
#include "subject.h"
#include <algorithm>

namespace engine::interface {

Observer::~Observer() {
    clearSubjects();
}

void Observer::addSubject(Subject* subject) {
    if (!subject) return;
    subjects_.push_back(subject);
}

void Observer::removeSubject(Subject* subject) {
    if (!subject) return;
    subjects_.erase(std::remove(subjects_.begin(), subjects_.end(), subject), subjects_.end());
}

void Observer::clearSubjects() {
    // 复制列表，避免在迭代时修改原列表
    auto subjectsCopy = subjects_;
    for (auto* subject : subjectsCopy) {
        if (subject) {
            subject->removeObserver(this);
        }
    }
    subjects_.clear();
}

} // namespace engine::interface