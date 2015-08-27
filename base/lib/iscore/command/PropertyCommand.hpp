#pragma once
#include <iscore/command/SerializableCommand.hpp>
#include <iscore/tools/ObjectPath.hpp>
#define ISCORE_PROPERTY_COMMAND_DEFAULT_CTOR(THE_CLASS, ParentName) THE_CLASS () : iscore::PropertyCommand{ ParentName , commandName(), description()} { }
namespace iscore
{
    class PropertyCommand : public SerializableCommand
    {
        public:
            using SerializableCommand::SerializableCommand;
            template<typename Path_T, typename... Args>
            PropertyCommand(Path_T&& path,
                            const QString& property,
                            const QVariant& newval,
                            Args&&... args):
                SerializableCommand{std::forward<Args>(args)...},
                m_path{std::move(std::move(path).moveUnsafePath())},
                m_property{property},
                m_new{newval}
            {
                m_old = m_path.find<QObject>().property(m_property.toLatin1().constData());
            }


            void undo() override;
            void redo() override;

            template<typename Path_T>
            void update(const Path_T&, const QVariant& newval)
            {
                m_new = newval;
            }

        protected:
            void serializeImpl(QDataStream &) const override;
            void deserializeImpl(QDataStream &) override;

        private:
            ObjectPath m_path;
            QString m_property;
            QVariant m_old, m_new;
    };
}
