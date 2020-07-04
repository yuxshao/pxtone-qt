#include "Clipboard.h"

#include <QDebug>
#include <algorithm>
Clipboard::Clipboard(const pxtnService *pxtn) : m_pxtn(pxtn) {}

// TODO: Make this adjustable
static const std::set<EVENTKIND> kinds_to_copy{
    EVENTKIND_ON, EVENTKIND_KEY, EVENTKIND_VELOCITY, EVENTKIND_VOLUME,
    EVENTKIND_PORTAMENT};

// TODO: Maybe also be able to copy the tails of ONs, the existing state for
// state kinds, and unset them at the end of the interval.
void Clipboard::copy(const std::set<int> &unit_nos, const Interval &range) {
  m_copy_length = range.length();
  m_items.clear();

  auto min = std::min_element(unit_nos.begin(), unit_nos.end());
  uint8_t first_unit_no = (min == unit_nos.end() ? 0 : *min);
  m_unit_nos.clear();
  for (const int &i : unit_nos) m_unit_nos.insert(i - first_unit_no);

  const EVERECORD *e = nullptr;
  for (e = m_pxtn->evels->get_Records(); e; e = e->next)
    if (e->clock >= range.start) break;

  for (; e && e->clock < range.end; e = e->next) {
    EVENTKIND kind(EVENTKIND(e->kind));
    if (unit_nos.find(e->unit_no) != unit_nos.end() &&
        kinds_to_copy.find(kind) != kinds_to_copy.end()) {
      int32_t v = e->value;
      if (Evelist_Kind_IsTail(e->kind)) v = std::min(v, range.end - e->clock);
      uint8_t unit_no = e->unit_no - first_unit_no;
      m_items.emplace_back(Item{e->clock - range.start, unit_no, kind, v});
    }
  }
}

std::list<Action::Primitive> Clipboard::makePaste(
    const std::set<int> &paste_unit_nos, qint32 start_clock,
    const UnitIdMap &map) {
  using namespace Action;
  std::list<Primitive> actions;
  auto min = std::min_element(paste_unit_nos.begin(), paste_unit_nos.end());
  if (min == paste_unit_nos.end()) return actions;
  uint8_t first_unit_no = *min;

  qint32 end_clock = start_clock + m_copy_length;
  for (const int &source_unit_no : m_unit_nos) {
    uint8_t unit_no = source_unit_no + first_unit_no;
    if (paste_unit_nos.find(unit_no) != paste_unit_nos.end()) {
      if (map.numUnits() <= unit_no) continue;
      qint32 unit_id = map.noToId(unit_no);
      for (const EVENTKIND &kind : kinds_to_copy)
        actions.emplace_back(
            Primitive{kind, unit_id, start_clock, Delete{end_clock}});
    }
  }

  for (const Item &item : m_items) {
    uint8_t unit_no = item.unit_no + first_unit_no;
    if (paste_unit_nos.find(unit_no) != paste_unit_nos.end() &&
        kinds_to_copy.find(item.kind) != kinds_to_copy.end()) {
      if (map.numUnits() <= unit_no) continue;
      qint32 unit_id = map.noToId(unit_no);
      actions.emplace_back(Primitive{
          item.kind, unit_id, start_clock + item.clock, Add{item.value}});
    }
  }
  return actions;
}

// TODO: Maybe shouldn't be here? Doesn't actually use clipboard state ATM
std::list<Action::Primitive> Clipboard::makeClear(const std::set<int> &unit_nos,
                                                  const Interval &range,
                                                  const UnitIdMap &map) {
  using namespace Action;
  std::list<Primitive> actions;
  // TODO: Dedup with makePaste
  for (const int &unit_no : unit_nos) {
    if (map.numUnits() <= unit_no) continue;
    qint32 unit_id = map.noToId(unit_no);
    for (const EVENTKIND &kind : kinds_to_copy)
      actions.emplace_back(
          Primitive{kind, unit_id, range.start, Delete{range.end}});
  }
  return actions;
}