import React, { useCallback, useState, ReactNode } from "react"
import { Settings, SWFType } from './PublisherSettings';

interface ContextProps {
    backwardCompatibility: boolean
    fileType: SWFType
}

interface ContextInterface {
    useBackwardCompatibility: boolean
    toggleBackwardCompatibility: () => void

    fileType: SWFType
    setFileType: (type: SWFType) => void
}

//@ts-ignore
const Context = React.createContext<ContextInterface>(null);

export const CreatePublishAppContext = function (props: ContextProps): ContextInterface {
    const [backwardCompatibility, setBackwardCompatibility] = useState(props.backwardCompatibility);
    const [fileType, setFileType] = useState(props.fileType);

    const backwardCompatibilitySetter = () =>
    {
        setBackwardCompatibility(backwardCompatibility == false)

        Settings.setParam("backwardCompatibility", backwardCompatibility == false)
    }

    const fileTypeSetter = (type: SWFType) =>
    {
        setFileType(type)
        Settings.setParam("type", type)
    }

    return {
        useBackwardCompatibility: backwardCompatibility,
        toggleBackwardCompatibility: backwardCompatibilitySetter,

        fileType: fileType,
        setFileType: fileTypeSetter
    };
}

export function UpdateContext()
{
    const {useBackwardCompatibility, toggleBackwardCompatibility, setFileType } = GetPublishContext();

    setFileType(Settings.getParam("type"))
    if (Settings.getParam("backwardCompatibility") != useBackwardCompatibility)
    {
        toggleBackwardCompatibility()
    }
}

//@ts-ignore
export const PublisherContextProvider = ({ children, ...props }) => {
    const context = CreatePublishAppContext(props as ContextProps);
    return <Context.Provider value={context}>{children}</Context.Provider>;
};

export function GetPublishContext() {
    const context = React.useContext(Context);
    if (!context) throw new Error('Failed to get context');
    return context;
}