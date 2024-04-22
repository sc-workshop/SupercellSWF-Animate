import React, { useCallback, useState, ReactNode } from "react"

interface ContextProps {
    backwardCompatibility: boolean
}

interface ContextInterface {
    useBackwardCompatibility: boolean
    toggleBackwardCompatibility: () => void
}

//@ts-ignore
const Context = React.createContext<ContextInterface>(null);

export const CreatePublishAppContext = function (props: ContextProps): ContextInterface {
    const [backwardCompatibility, setBackwardCompatibility] = useState(props.backwardCompatibility);

    const toggleBackwardCompatibility = () =>
    {
        setBackwardCompatibility(backwardCompatibility == false)
    }

    return {
        useBackwardCompatibility: backwardCompatibility,
        toggleBackwardCompatibility: toggleBackwardCompatibility,
    };
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