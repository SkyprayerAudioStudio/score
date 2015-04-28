#include "GroupManager.hpp"
#include "Group.hpp"
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/serialization/JSONVisitor.hpp>


template<>
void Visitor<Reader<DataStream>>::readFrom(const GroupManager& elt)
{
    readFrom(static_cast<const IdentifiedObject<GroupManager>&>(elt));
    const auto& groups = elt.groups();
    m_stream << (int) groups.size();
    for(const auto& group : groups)
    {
        readFrom(*group);
    }

    insertDelimiter();
}

template<>
void Visitor<Writer<DataStream>>::writeTo(GroupManager& elt)
{
    int size;
    m_stream >> size;
    for(int i = size; i --> 0; )
    {
        elt.addGroup(new Group{*this, &elt});
    }
    checkDelimiter();
}


template<>
void Visitor<Reader<JSONObject>>::readFrom(const GroupManager& elt)
{
    readFrom(static_cast<const IdentifiedObject<GroupManager>&>(elt));
    m_obj["Groups"] = toJsonArray(elt.groups());
}


template<>
void Visitor<Writer<JSONObject>>::writeTo(GroupManager& elt)
{
    auto arr = m_obj["Groups"].toArray();
    for(const auto& json_vref : arr)
    {
        Deserializer<JSONObject> deserializer {json_vref.toObject()};
        elt.addGroup(new Group{deserializer, &elt});
    }
}
