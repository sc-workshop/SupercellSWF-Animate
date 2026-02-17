#pragma once

#include "AnimatePublisher.h"
#include "Window/Components/StatusComponent.h"
#include "flash/flash.h"

namespace sc::Adobe {
    class SCWriter;

    class SCMovieclipWriter : public Animate::Publisher::SharedMovieclipWriter {
    private:
        SCWriter& m_writer;
        StatusComponent* m_status = nullptr;

        // Current object
        flash::MovieClip m_object;

        // Bank with current object transformations only
        flash::MatrixBank m_bank;

        // Helper functions

        uint16_t GetInstanceIndex(uint16_t elementsCount, uint16_t id, FCM::BlendMode blending, std::string name);

        void FinalizeTransforms();

    public:
        SCMovieclipWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol);
        virtual ~SCMovieclipWriter();
        virtual void Next();
        virtual void InitializeTimeline(uint32_t fps, uint32_t frameCount);
        virtual void SetLabel(const std::u16string& label);
        virtual void AddFrameElement(Animate::Publisher::ResourceReference resource,
                                     FCM::BlendMode blending,
                                     const std::u16string& name,
                                     std::optional<Animate::DOM::Utils::MATRIX2D> matrix,
                                     std::optional<Animate::DOM::Utils::COLOR_MATRIX> color);

        virtual bool Finalize(Animate::Publisher::ResourceReference reference, bool required, bool new_symbol);

    protected:
        virtual std::size_t GenerateHash() const;
    };
}