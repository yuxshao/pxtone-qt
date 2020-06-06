#include "PxtoneActionSynchronizer.h"

#include <QDebug>
PxtoneActionSynchronizer::PxtoneActionSynchronizer(int uid, pxtnEvelist *evels)
    : m_uid(uid), m_evels(evels), m_local_index(0), m_remote_index(0) {}

RemoteAction PxtoneActionSynchronizer::applyLocalAction(
    const std::vector<Action> &action) {
  m_uncommitted.push_back(apply_actions_and_get_undo(action, m_evels));
  qDebug() << "Remote" << m_remote_index << "Local" << m_local_index;
  qDebug() << "New action";
  return RemoteAction{RemoteAction::Type::ACTION, m_local_index++, action};
}

RemoteAction PxtoneActionSynchronizer::getUndo() {
  return RemoteAction{RemoteAction::Type::UNDO, m_local_index++,
                      std::vector<Action>()};
}
RemoteAction PxtoneActionSynchronizer::getRedo() {
  return RemoteAction{RemoteAction::Type::REDO, m_local_index++,
                      std::vector<Action>()};
}

void PxtoneActionSynchronizer::setUid(qint64 uid) { m_uid = uid; }

void PxtoneActionSynchronizer::applyRemoteAction(
    const RemoteActionWithUid &actionWithUid) {
  const RemoteAction &action = actionWithUid.action;
  qint64 uid = actionWithUid.uid;
  qDebug() << "Remote" << m_remote_index << "Local" << m_local_index;
  qDebug() << "Received action" << action.idx << "from user" << uid << "type"
           << action.type;
  if (uid == m_uid) {
    if (action.idx != m_remote_index) {
      qWarning() << "Received a remote action with index" << action.idx
                 << "that was not the one I expected" << m_remote_index
                 << ". Discarding. ";
      return;
    }
    ++m_remote_index;
  }
  switch (action.type) {
    case RemoteAction::Type::ACTION:
      if (uid == m_uid) {
        // TODO: Respond to unexpected indexes smarter. What would make things
        // consistent?
        if (m_uncommitted.size() == 0) {
          qWarning() << "Received a remote action of mine even when I'm not "
                        "expecting any pending remote responses. Discarding. "
                     << action.idx << m_remote_index;
        } else {
          // The server told us that our local action was applied! Put it in the
          // log, but no need to apply any actions since that was already
          // presumptuously applied.

          m_log.emplace_back(uid, action.idx, m_uncommitted.front());
          m_uncommitted.pop_front();
        }
      } else {
        // Undo each of the uncommitted actions
        // TODO: This is probably where you could be smarter and avoid undoing
        // things that don't intersect in bbox
        for (auto uncommitted = m_uncommitted.rbegin();
             uncommitted != m_uncommitted.rend(); ++uncommitted) {
          *uncommitted = apply_actions_and_get_undo(*uncommitted, m_evels);
        }

        // apply the committed action
        std::vector<Action> reverse =
            apply_actions_and_get_undo(action.action, m_evels);

        // redo each of the uncommitted actions forwards
        for (std::vector<Action> &uncommitted : m_uncommitted) {
          uncommitted = apply_actions_and_get_undo(uncommitted, m_evels);
        }

        m_log.emplace_back(uid, action.idx, reverse);
      }

      // Invalidate any previous undone actions by this user
      for (auto it = m_log.rbegin(); it != m_log.rend(); ++it) {
        if (it->uid == uid) switch (it->state) {
            case LoggedAction::UNDONE:
              it->state = LoggedAction::GONE;
              break;
            case LoggedAction::GONE:
              break;
            case LoggedAction::DONE:
              // break out of the outer for with a goto
              goto exit_loop;
          }
      }
    exit_loop:
      // qDebug() << "m_log size" << m_log.size();
      break;
    case RemoteAction::Type::UNDO:
    case RemoteAction::Type::REDO:
      qDebug() << "Applying undo / redo";
      for (auto uncommitted = m_uncommitted.rbegin();
           uncommitted != m_uncommitted.rend(); ++uncommitted) {
        *uncommitted = apply_actions_and_get_undo(*uncommitted, m_evels);
      }

      // Go back to last {undone,done} action by user, temporarily undoing done
      // actions by other users. Flip. Then redo the done actions by other
      // users.
      {
        auto flip_from =
            (action.type == RemoteAction::UNDO ? LoggedAction::DONE
                                               : LoggedAction::UNDONE);
        auto flip_to = (action.type == RemoteAction::UNDO ? LoggedAction::UNDONE
                                                          : LoggedAction::DONE);

        // qDebug() << "m_log size" << m_log.size();
        std::list<LoggedAction *> temporarily_undone;
        for (auto it = m_log.rbegin(); it != m_log.rend(); ++it) {
          if (it->state == flip_from && it->uid == uid) {
            qDebug() << "Flipped " << it->uid << it->idx;
            it->reverse = apply_actions_and_get_undo(it->reverse, m_evels);
            it->state = flip_to;
            break;
          } else if (it->state == LoggedAction::UndoState::DONE) {
            qDebug() << "Temporarily undoing " << it->uid << it->idx;
            it->reverse = apply_actions_and_get_undo(it->reverse, m_evels);
            temporarily_undone.push_front(&(*it));
          }
        }
        for (LoggedAction *it : temporarily_undone)
          it->reverse = apply_actions_and_get_undo(it->reverse, m_evels);
      }

      for (std::vector<Action> &uncommitted : m_uncommitted) {
        uncommitted = apply_actions_and_get_undo(uncommitted, m_evels);
      }
  }
}