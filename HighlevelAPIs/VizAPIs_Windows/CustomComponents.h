#pragma once

#include <utils/EntityManager.h>
#include <utils/EntityInstance.h>
#include <utils/NameComponentManager.h>

// NOTE THAT ALL ENTITIES ARE SUPPOSED TO HAVE NAME COMPONENTS
namespace vzm
{
    class VzNameCompManager : utils::NameComponentManager
    {
    private:
        std::unordered_multimap<std::string, utils::Entity> nameToEntities_;
    public:

        using Instance = utils::EntityInstance<NameComponentManager>;


        //explicit NameComponentManager(EntityManager& em);
        explicit VzNameCompManager(utils::EntityManager& em) : 
            utils::NameComponentManager(em) {
        }

        void CreateNameComp(const utils::Entity ett, const std::string& name)
        {
            addComponent(ett);
            setName(getInstance(ett), name.c_str());

            nameToEntities_.emplace(name, ett);
        }
        void RemoveEntity(utils::Entity ett)
        {
            auto ins = getInstance(ett);
            if (ins.asValue() == 0)
            {
                return;
            }

            std::string name(getName(ins));
            auto range = nameToEntities_.equal_range(name);
            for (auto it = range.first; it != range.second; ++it) {
                if (it->second == ett)
                {
                    nameToEntities_.erase(it);
                    break;
                }
            }

            removeComponent(ett);
        }
        std::vector<utils::Entity> GetEntitiesByName(const std::string& name) const
        {
            std::vector<utils::Entity> result;
            auto range = nameToEntities_.equal_range(name);
            for (auto it = range.first; it != range.second; ++it) {
                result.push_back(it->second);
            }
            return result;
        }
        utils::Entity GetFirstEntityByName(const std::string& name) const
        {
            auto it = nameToEntities_.find(name);
            if (it == nameToEntities_.end())
            {
                return utils::Entity();
            }
            return it->second;
        }
        std::string GetName(utils::Entity ett)
        {
            auto ins = getInstance(ett);
            if (ins.asValue() == 0)
            {
                return "";
            }
            return std::string(getName(ins));
        }
        static VzNameCompManager& Get(const bool init = false) noexcept
        {
            static VzNameCompManager* ncm = new (std::nothrow) VzNameCompManager(utils::EntityManager::get());
            return *ncm;
        }
    };
}
