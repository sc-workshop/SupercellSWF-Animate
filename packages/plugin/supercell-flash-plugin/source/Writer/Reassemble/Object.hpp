#pragma once

#include "flash/flash.h"

#include <fstream>
#include <iostream>

namespace sc::flash {
    using ReferenceMap = std::map<uint16_t, uint32_t>;
    using IdMap = std::map<uint16_t, uint16_t>;

    static void get_object_references(SupercellSWF& swf, ReferenceMap& object_references, uint16_t id) {
        auto add_reference = [&object_references](uint16_t id) {
            if (!object_references.count(id)) {
                object_references[id] = 1;
            } else {
                object_references[id] += 1;
            }
        };

        std::function<void(uint16_t)> process_display_object =
            [&swf, &add_reference, &process_display_object](uint16_t id) {
                add_reference(id);

                DisplayObject& object = swf.GetDisplayObjectByID(id);
                if (object.is_movieclip()) {
                    MovieClip& movieclip = (MovieClip&) object;
                    for (DisplayObjectInstance instance : movieclip.childrens) {
                        process_display_object(instance.id);
                    }
                }
            };

        DisplayObject& object = swf.GetDisplayObjectByID(id);
        add_reference(id);
        if (object.is_movieclip()) {
            for (DisplayObjectInstance& instance : ((MovieClip&) object).childrens) {
                process_display_object(instance.id);
            }
        }
    }

    static void erase_objects_if(SupercellSWF& swf, std::function<bool(const DisplayObject&)> condition) {
        swf.movieclips.erase(std::remove_if(swf.movieclips.begin(), swf.movieclips.end(), condition),
                             swf.movieclips.end());

        swf.shapes.erase(std::remove_if(swf.shapes.begin(), swf.shapes.end(), condition), swf.shapes.end());

        swf.movieclip_modifiers.erase(std::remove_if(swf.movieclip_modifiers.begin(),
                                                     swf.movieclip_modifiers.end(),
                                                     condition),
                                      swf.movieclip_modifiers.end());

        swf.textfields.erase(std::remove_if(swf.textfields.begin(), swf.textfields.end(), condition),
                             swf.textfields.end());
    }

    static void remove_unused(SupercellSWF& swf) {
        ReferenceMap object_references;

        auto is_object_has_reference = [&object_references](const DisplayObject& object) {
            return !object_references.count(object.id);
        };

        // Collecting object references count
        for (ExportName& export_name : swf.exports) {
            get_object_references(swf, object_references, export_name.id);
        }

        // Erasing unused objects
        erase_objects_if(swf, is_object_has_reference);
    }
}