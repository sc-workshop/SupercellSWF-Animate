import { type CSSProperties, useRef } from "react";

type IconButtonProps = {
	src: string;
	alt: string;
	onClick(): void;
	rotate?: boolean;
	active?: boolean;
	size?: number;
	padding?: number;
	borderRadius?: number;
	style?: CSSProperties;
};

export default function IconButton({
	src,
	alt,
	onClick,
	rotate = false,
	active = false,
	size = 28,
	padding = 4,
	borderRadius = 4,
	style = {},
}: IconButtonProps) {
	const buttonRef = useRef<HTMLButtonElement>(null);

	const handleMouseDown = () => {
		if (buttonRef.current) buttonRef.current.style.transform += " scale(0.9)";
	};
	const handleMouseUp = () => {
		if (buttonRef.current)
			buttonRef.current.style.transform = rotate
				? active
					? "rotate(180deg)"
					: "rotate(0)"
				: "";
	};

	return (
		<button
			ref={buttonRef}
			onClick={onClick}
			type="button"
			style={{
				width: `${size}px`,
				height: `${size}px`,
				cursor: "pointer",
				background: "none",
				border: "none",
				borderRadius: `${borderRadius}px`,
				padding: `${padding}px`,
				display: "flex",
				alignItems: "center",
				justifyContent: "center",
				transition: "background 120ms, transform 150ms ease",
				transform: rotate ? (active ? "rotate(180deg)" : "rotate(0)") : "",
				...style,
			}}
			onMouseEnter={(e) => {
				e.currentTarget.style.background = "rgba(255,255,255,0.1)";
			}}
			onMouseLeave={(e) => {
				e.currentTarget.style.background = "none";
				handleMouseUp();
			}}
			onMouseDown={handleMouseDown}
			onMouseUp={handleMouseUp}
		>
			<img
				src={src}
				alt={alt}
				style={{
					width: "100%",
					height: "100%",
					opacity: 0.8,
				}}
			/>
		</button>
	);
}
