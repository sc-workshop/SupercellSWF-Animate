/**
 * @class ContextMenuItemStatus
 * Stores the status of the context menu item.
 *
 * Since 5.2.0
 */
/** biome-ignore-all lint/complexity/noStaticOnlyClass: Interfaces with static variables */
export class ContextMenuItemStatus {
	/**
	 * Stores the status of the context menu item.
	 *
	 * Since 5.2.0
	 *
	 * @param menuItemID     The menu item id.
	 * @param enabled  		 True if user wants to enable the menu item.
	 * @param checked  		 True if user wants to check the menu item.
	 */
	constructor(
		public menuItemID: string,
		public enabled: boolean,
		public checked: boolean,
	) {}
}

/**
 * @class MenuItemStatus
 * Stores flyout menu item status
 *
 * Since 5.2.0
 */
export class MenuItemStatus {
	/**
	 * Stores flyout menu item status
	 *
	 * Since 5.2.0
	 *
	 * @param menuItemLabel  The menu item label.
	 * @param enabled  		 True if user wants to enable the menu item.
	 * @param checked  		 True if user wants to check the menu item.
	 */
	constructor(
		public menuItemLabel: string,
		public enabled: boolean,
		public checked: boolean,
	) {}
}

/**
 * @class SystemPath
 * Stores operating-system-specific location constants for use in the
 * \c #CSInterface.getSystemPath() method.
 */
export class SystemPath {
	/** The path to user data.  */
	public static readonly USER_DATA: string = "userData";

	/** The path to common files for Adobe applications.  */
	public static readonly COMMON_FILES: string = "commonFiles";

	/** The path to the user's default document folder.  */
	public static readonly MY_DOCUMENTS: string = "myDocuments";

	/** @deprecated. Use \c #SystemPath.Extension.  */
	public static readonly APPLICATION: string = "application";

	/** The path to current extension.  */
	public static readonly EXTENSION: string = "extension";

	/** The path to hosting application's executable.  */
	public static readonly HOST_APPLICATION: string = "hostApplication";
}

/**
 * @class ColorType
 * Stores color-type constants.
 */
export class ColorType {
	/** RGB color type. */
	public static readonly RGB: string = "rgb";

	/** Gradient color type. */
	public static readonly GRADIENT: string = "gradient";

	/** Null color type. */
	public static readonly NONE: string = "none";
}

/**
 * @class RGBColor
 * Stores an RGB color with red, green, blue, and alpha values.
 * All values are in the range [0.0 to 255.0]. Invalid numeric values are
 * converted to numbers within this range.
 *
 */
export class RGBColor {
	/**
	 * Stores an RGB color with red, green, blue, and alpha values.
	 * All values are in the range [0.0 to 255.0]. Invalid numeric values are
	 * converted to numbers within this range.
	 *
	 * @param red   The red value, in the range [0.0 to 255.0].
	 * @param green The green value, in the range [0.0 to 255.0].
	 * @param blue  The blue value, in the range [0.0 to 255.0].
	 * @param alpha The alpha (transparency) value, in the range [0.0 to 255.0].
	 *      The default, 255.0, means that the color is fully opaque.
	 */
	constructor(
		public red: number,
		public green: number,
		public blue: number,
		public alpha: number,
	) {}
}

/**
 * @class Direction
 * A point value  in which the y component is 0 and the x component
 * is positive or negative for a right or left direction,
 * or the x component is 0 and the y component is positive or negative for
 * an up or down direction.
 */
export class Direction {
	/**
	 * A point value  in which the y component is 0 and the x component
	 * is positive or negative for a right or left direction,
	 * or the x component is 0 and the y component is positive or negative for
	 * an up or down direction.
	 *
	 * @param x     The horizontal component of the point.
	 * @param y     The vertical component of the point.
	 */
	constructor(
		public x: number,
		public y: number,
	) {}
}

/**
 * @class GradientStop
 * Stores gradient stop information.
 */
export class GradientStop {
	/**
	 * Stores gradient stop information.
	 *
	 * @param offset   The offset of the gradient stop, in the range [0.0 to 1.0].
	 * @param rgbColor The color of the gradient at this point, an \c #RGBColor object.
	 */
	constructor(
		public offset: number,
		public rgbColor: RGBColor,
	) {}
}

/**
 * @class GradientColor
 * Stores gradient color information.
 */
export class GradientColor {
	/**
     * Initializes new \c #GradientColor instance.
     *
     * @param type          The gradient type, must be "linear".
     * @param direction     A \c #Direction object for the direction of the gradient
     (up, down, right, or left).
     * @param numStops          The number of stops in the gradient.
     * @param gradientStopList  An array of \c #GradientStop objects.
     */
	constructor(
		public type: string,
		public direction: Direction,
		public numStops: number,
		public arrGradientStop: GradientStop[],
	) {}
}

/**
 * @class UIColor
 * Stores color information, including the type, anti-alias level, and specific color
 * values in a color object of an appropriate type.
 */
export class UIColor {
	/**
     * Stores color information, including the type, anti-alias level, and specific color
     * values in a color object of an appropriate type.
     *
     * @param type          The color type, 1 for "rgb" and 2 for "gradient".
     The supplied color object must correspond to this type.
     * @param antialiasLevel    The anti-alias level constant.
     * @param color         A \c #RGBColor or \c #GradientColor object containing specific color information.
     */
	constructor(
		type?: number,
		antialiasLevel?: any,
		color?: RGBColor | GradientColor,
	) {
		this.type = type;
		this.antialiasLevel = antialiasLevel;
		this.color = color;
	}

	/**
	 * The color type, 1 for "rgb" and 2 for "gradient".
	 * The supplied color object must correspond to this type.
	 */
	public type?: number;

	/**
	 * The anti-alias level constant.
	 */
	public antialiasLevel: any;

	/**
	 * A \c #RGBColor or \c #GradientColor object containing specific color information.
	 */
	public color?: RGBColor | GradientColor;
}

/**
 * @class AppSkinInfo
 * Stores window-skin properties, such as color and font. All color parameter values are \c #UIColor objects except that systemHighlightColor is \c #RGBColor object.
 */
export class AppSkinInfo {
	/**
	 * Stores window-skin properties, such as color and font. All color parameter values are \c #UIColor objects except that systemHighlightColor is \c #RGBColor object.
	 *
	 * @param baseFontFamily        The base font family of the application.
	 * @param baseFontSize          The base font size of the application.
	 * @param appBarBackgroundColor     The application bar background color.
	 * @param panelBackgroundColor      The background color of the extension panel.
	 * @param appBarBackgroundColorSRGB     The application bar background color, as sRGB.
	 * @param panelBackgroundColorSRGB      The background color of the extension panel, as sRGB.
	 * @param systemHighlightColor          The operating-system highlight color, as sRGB.
	 */
	constructor(
		public baseFontFamily: string,
		public baseFontSize: number,
		public appBarBackgroundColor: UIColor,
		public panelBackgroundColor: UIColor,
		public appBarBackgroundColorSRGB: UIColor,
		public panelBackgroundColorSRGB: UIColor,
		public systemHighlightColor: UIColor,
	) {}
}

/**
 * @class HostEnvironment
 * Stores information about the environment in which the extension is loaded.
 */
export class HostEnvironment {
	/**
	 * Stores information about the environment in which the extension is loaded.
	 *
	 * @param appName   The application's name.
	 * @param appVersion    The application's version.
	 * @param appLocale The application's current license locale.
	 * @param appUILocale   The application's current UI locale.
	 * @param appId     The application's unique identifier.
	 * @param isAppOnline  True if the application is currently online.
	 * @param appSkinInfo   An \c #AppSkinInfo object containing the application's default color and font styles.
	 */
	constructor(
		public appName: string,
		public appVersion: any,
		public appLocale: any,
		public appUILocale: any,
		public appId: string,
		public isAppOnline: boolean,
		public appSkinInfo: AppSkinInfo,
	) {}
}

/**
 * @class HostCapabilities
 * Stores information about the host capabilities.
 */
export class HostCapabilities {
	/**
	 * Stores information about the host capabilities.
	 *
	 * @param EXTENDED_PANEL_MENU True if the application supports panel menu.
	 * @param EXTENDED_PANEL_ICONS True if the application supports panel icon.
	 * @param DELEGATE_APE_ENGINE True if the application supports delegated APE engine.
	 * @param SUPPORT_HTML_EXTENSIONS True if the application supports HTML extensions.
	 * @param DISABLE_FLASH_EXTENSIONS True if the application disables FLASH extensions.
	 */
	constructor(
		public EXTENDED_PANEL_MENU: boolean,
		public EXTENDED_PANEL_ICONS: boolean,
		public DELEGATE_APE_ENGINE: boolean,
		public SUPPORT_HTML_EXTENSIONS: boolean,
		public DISABLE_FLASH_EXTENSIONS: boolean,
	) {}
}

/**
 * @class ApiVersion
 * Stores current api version.
 *
 * Since 4.2.0
 */
export class ApiVersion {
	/**
	 * Stores current api version.
	 *
	 * Since 4.2.0
	 *
	 * @param major  The major version.
	 * @param minor  The minor version.
	 * @param micro  The micro version.
	 */
	constructor(
		public major: number,
		public minor: number,
		public micro: number,
	) {}
}
