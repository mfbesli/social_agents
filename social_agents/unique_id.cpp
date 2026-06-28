#include "unique_id.h"

unique_id::records::records() : reg(), next(1) {}

unique_id::holder::holder(records& r) : rec(&r), id(r.assign(*this)) {}

unique_id::holder::~holder() { rec->erase(id); }

bool unique_id::compare::operator()(const holder& lhs, const holder& rhs) const { return lhs.id < rhs.id; }

