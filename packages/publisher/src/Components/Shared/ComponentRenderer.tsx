import DisplayObject from "Components/Shared/DisplayObject";
import { GetPublishContext } from "Publisher/Context";
import React, { type CSSProperties, useEffect, useState } from "react";

function AnimatedItem({
	content,
	visible,
	disableAnimation = false,
	transitionDuration = 300,
}: {
	content: React.ReactNode;
	visible: boolean;
	disableAnimation?: boolean;
	transitionDuration?: number;
}) {
	const [isVisible, setIsVisible] = useState(visible);
	const [rendered, setRendered] = useState(visible);

	useEffect(() => {
		if (disableAnimation) {
			setIsVisible(visible);
			setRendered(visible);
			return;
		}

		if (visible) {
			setRendered(true);
			setTimeout(() => setIsVisible(true), 10);
		} else {
			setIsVisible(false);
			const timeout = setTimeout(() => setRendered(false), transitionDuration);
			return () => clearTimeout(timeout);
		}
	}, [visible, disableAnimation, transitionDuration]);

	if (!rendered) return null;

	const style: CSSProperties = disableAnimation
		? { opacity: 1, transform: "translateY(0)", gap: "8px" }
		: {
				gap: "8px",
				opacity: isVisible ? 1 : 0,
				transform: isVisible ? "translateY(0)" : "translateY(-20px)",
				overflow: "hidden",
				transition: `opacity ${transitionDuration}ms ease, transform ${transitionDuration}ms ease`,
			};

	return <div style={style}>{content}</div>;
}

export default function renderComponents(
	components: any[],
	condition: boolean | undefined = undefined,
	transitionDuration: number = 100,
) {
	const disableAnimation = condition === undefined;

	const [visible, setVisible] = useState(
		disableAnimation ? true : (condition ?? false),
	);

	useEffect(() => {
		if (disableAnimation) {
			setVisible(true);
			return;
		}

		setVisible(condition ?? false);
	}, [condition, disableAnimation]);

	const containerStyle: CSSProperties = {
		display: "flex",
		flexDirection: "column",
		gap: "8px",
	};

	const result = (
		<div style={containerStyle}>
			{components.map((component, index) => {
				if (component === undefined) return undefined;

				let content: React.ReactNode;
				if (component && component instanceof React.Component) {
					content = component.render();

					if (component instanceof DisplayObject && component.IsAutoProperty) {
						const { useAutoProperties } = GetPublishContext();
						if (useAutoProperties) return null;
					}
				} else {
					content = component;
				}

				return (
					<AnimatedItem
						key={index}
						content={content}
						visible={visible}
						disableAnimation={disableAnimation}
						transitionDuration={transitionDuration}
					/>
				);
			})}
		</div>
	);

	if (condition === undefined) return result;

	return condition ? result : undefined;
}
