/**
 *  Typescript implementation of CSInterface - v7.0.0
 *
 *  Custom CSInterface.js implementation in Typescript.
 *  The implementation covers version 7.x from original repository, with all functionalities.
 *  All documentation comments were also based on original.
 *
 *
 *  For more information about Creative Cloud extensions development
 *  please refer to original Github page: https://github.com/Adobe-CEP/CEP-Resources
 */

import { ApiVersion, MenuItemStatus, ContextMenuItemStatus } from "./storages";

declare let cep: any;

declare global {
	interface Window {
		__adobe_cep__: any;
	}
}

/**
* @class CSInterface
* This is the entry point to the CEP extensibility infrastructure.
* Instantiate this object and use it to:
* <ul>
* <li>Access information about the host application in which an extension is running</li>
* <li>Launch an extension</li>
* <li>Register interest in event notifications, and dispatch events</li>
* </ul>
*
*/
export class CSInterface {
  /**
   * User can add this event listener to handle native application theme color changes.
   * Callback function gives extensions ability to fine-tune their theme color after the
   * global theme color has been changed.
   * The callback function should be like below:
   *
   * @example
   * // event is a CSEvent object, but user can ignore it.
   * function OnAppThemeColorChanged(event)
   * {
   *    // Should get a latest HostEnvironment object from application.
   *    var skinInfo = JSON.parse(window.__adobe_cep__.getHostEnvironment()).appSkinInfo;
   *    // Gets the style information such as color info from the skinInfo,
   *    // and redraw all UI controls of your extension according to the style info.
   * }
   */
  public readonly THEME_COLOR_CHANGED_EVENT: string = "com.adobe.csxs.events.ThemeColorChanged";

  /** The host environment data object. */
  public hostEnvironment: any = JSON.parse(window.__adobe_cep__.getHostEnvironment());

  /** Retrieves information about the host environment in which the
   *  extension is currently running.
   *
   *   @return A \c #HostEnvironment object.
   */
  getHostEnvironment(): {
    appUILocale: string
    appLocale: string
    appVersion: string
    appName: string
  } {
    this.hostEnvironment = JSON.parse(window.__adobe_cep__.getHostEnvironment());
    return this.hostEnvironment;
  };

  /** Closes this extension. */
  closeExtension() {
    window.__adobe_cep__.closeExtension();
  };

  /**
   * Retrieves a path for which a constant is defined in the system.
   *
   * @param pathType The path-type constant defined in \c #SystemPath ,
   *
   * @return The platform-specific system path string.
   */
  getSystemPath(pathType: string): string {
    let path = decodeURI(window.__adobe_cep__.getSystemPath(pathType));
    const OSVersion = this.getOSInformation();
    if (OSVersion.indexOf("Windows") >= 0) {
      path = path.replace("file:///", "");
    }
    else if (OSVersion.indexOf("Mac") >= 0) {
      path = path.replace("file://", "");
    }
    return path;
  };

  /**
   * Evaluates a JavaScript script, which can use the JavaScript DOM
   * of the host application.
   *
   * @param script    The JavaScript script.
   * @param callback  Optional. A callback function that receives the result of execution.
   *          If execution fails, the callback function receives the error message \c EvalScript_ErrMessage.
   */
  evalScript(script: string): Promise<string> {
    return new Promise(function(resolve){
      window.__adobe_cep__.evalScript(script, function(value: string){resolve(value)});
    })
    
  };

  /**
   * Retrieves the unique identifier of the application.
   * in which the extension is currently running.
   *
   * @return The unique ID string.
   */
  getApplicationID(): string {
    const appId = this.hostEnvironment.appId;
    return appId;
  };


  /**
   * Retrieves host capability information for the application
   * in which the extension is currently running.
   *
   * @return A \c #HostCapabilities object.
   */
  getHostCapabilities(): any {
    const hostCapabilities = JSON.parse(window.__adobe_cep__.getHostCapabilities());
    return hostCapabilities;
  };

  /**
   * Triggers a CEP event programmatically. Yoy can use it to dispatch
   * an event of a predefined type, or of a type you have defined.
   *
   * @param event A \c CSEvent object.
   */
  dispatchEvent(event: CSEvent) {
    if (typeof event.data == "object") {
      event.data = JSON.stringify(event.data);
    }

    window.__adobe_cep__.dispatchEvent(event);
  };

  /**
   * Registers an interest in a CEP event of a particular type, and
   * assigns an event handler.
   * The event infrastructure notifies your extension when events of this type occur,
   * passing the event object to the registered handler function.
   *
   * @param type     The name of the event type of interest.
   * @param listener The JavaScript handler function or method.
   * @param obj      Optional, the object containing the handler method, if any.
   *         Default is null.
   */
  addEventListener(type: string, listener: any, obj?: any) {
    window.__adobe_cep__.addEventListener(type, listener, obj);
  };

  /**
   * Removes a registered event listener.
   *
   * @param type      The name of the event type of interest.
   * @param listener  The JavaScript handler function or method that was registered.
   * @param obj       Optional, the object containing the handler method, if any.
   *          Default is null.
   */
  removeEventListener(type: string, listener: any, obj?: any) {
    window.__adobe_cep__.removeEventListener(type, listener, obj);
  };

  /**
   * Loads and launches another extension, or activates the extension if it is already loaded.
   *
   * @param extensionId       The extension's unique identifier.
   * @param startupParams     Not currently used, pass "".
   *
   * @example
   * To launch the extension "help" with ID "HLP" from this extension, call:
   * <code>requestOpenExtension("HLP", ""); </code>
   *
   */
  requestOpenExtension(extensionId: string, params: string) {
    window.__adobe_cep__.requestOpenExtension(extensionId, params);
  };

  /**
   * Retrieves the list of extensions currently loaded in the current host application.
   * The extension list is initialized once, and remains the same during the lifetime
   * of the CEP session.
   *
   * @param extensionIds  Optional, an array of unique identifiers for extensions of interest.
   *          If omitted, retrieves data for all extensions.
   *
   * @return Zero or more \c #Extension objects.
   */
  getExtensions(extensionIds: string[]): any {
    const extensionIdsStr = JSON.stringify(extensionIds);
    const extensionsStr = window.__adobe_cep__.getExtensions(extensionIdsStr);

    const extensions = JSON.parse(extensionsStr);
    return extensions;
  };

  /**
   * Retrieves network-related preferences.
   *
   * @return A JavaScript object containing network preferences.
   */
  getNetworkPreferences(): any {
    const result = window.__adobe_cep__.getNetworkPreferences();
    const networkPre = JSON.parse(result);

    return networkPre;
  };

  /**
   * Initializes the resource bundle for this extension with property values
   * for the current application and locale.
   * To support multiple locales, you must define a property file for each locale,
   * containing keyed display-string values for that locale.
   * See localization documentation for Extension Builder and related products.
   *
   * Keys can be in the
   * form <code>key.value="localized string"</code>, for use in HTML text elements.
   * For example, in this input element, the localized \c key.value string is displayed
   * instead of the empty \c value string:
   *
   * <code><input type="submit" value="" data-locale="key"/></code>
   *
   * @return An object containing the resource bundle information.
   */
  /* initResourceBundle(): any {
    const resourceBundle = JSON.parse(window.__adobe_cep__.initResourceBundle());
    const resElms = document.querySelectorAll('[data-locale]');
    for (let n = 0; n < resElms.length; n++) {
      const resEl = resElms[n];
      // Get the resource key from the element.
      const resKey = resEl.getAttribute('data-locale');
      if (resKey) {
        // Get all the resources that start with the key.
        for (const key in resourceBundle) {
          if (key.indexOf(resKey) === 0) {
            const resValue = resourceBundle[key];
            if (key.length === resKey.length) {
              resEl.innerHTML = resValue;
            }
            else if ('.' === key.charAt(resKey.length)) {
              const attrKey = key.substring(resKey.length + 1);
              resEl[attrKey] = resValue;
            }
          }
        }
      }
    }
    return resourceBundle;
  }; */

  /**
   * Writes installation information to a file.
   *
   * @return The file path.
   */
  dumpInstallationInfo(): string {
    return window.__adobe_cep__.dumpInstallationInfo();
  };

  /**
   * Retrieves version information for the current Operating System,
   * See http://www.useragentstring.com/pages/Chrome/ for Chrome \c navigator.userAgent values.
   *
   * @return A string containing the OS version, or "unknown Operation System".
   * If user customizes the User Agent by setting CEF command parameter "--user-agent", only
   * "Mac OS X" or "Windows" will be returned.
   */
  getOSInformation(): string {
    const userAgent = navigator.userAgent;

    if ((navigator.platform === "Win32") || (navigator.platform === "Windows")) {
      let winVersion = "Windows";
      let winBit = "";
      if (userAgent.indexOf("Windows") > -1) {
        if (userAgent.indexOf("Windows NT 5.0") > -1) {
          winVersion = "Windows 2000";
        }
        else if (userAgent.indexOf("Windows NT 5.1") > -1) {
          winVersion = "Windows XP";
        }
        else if (userAgent.indexOf("Windows NT 5.2") > -1) {
          winVersion = "Windows Server 2003";
        }
        else if (userAgent.indexOf("Windows NT 6.0") > -1) {
          winVersion = "Windows Vista";
        }
        else if (userAgent.indexOf("Windows NT 6.1") > -1) {
          winVersion = "Windows 7";
        }
        else if (userAgent.indexOf("Windows NT 6.2") > -1) {
          winVersion = "Windows 8";
        }
        else if (userAgent.indexOf("Windows NT 6.3") > -1) {
          winVersion = "Windows 8.1";
        }
        else if (userAgent.indexOf("Windows NT 10") > -1) {
          winVersion = "Windows 10";
        }

        if (userAgent.indexOf("WOW64") > -1 || userAgent.indexOf("Win64") > -1) {
          winBit = " 64-bit";
        }
        else {
          winBit = " 32-bit";
        }
      }

      return winVersion + winBit;
    }
    else if ((navigator.platform === "MacIntel") || (navigator.platform === "Macintosh")) {
      let result = "Mac OS X";

      if (userAgent.indexOf("Mac OS X") > -1) {
        result = userAgent.substring(userAgent.indexOf("Mac OS X"), userAgent.indexOf(")"));
        result = result.replace(/_/g, ".");
      }

      return result;
    }

    return "Unknown Operation System";
  };

  /**
   * Opens a page in the default system browser.
   *
   * Since 4.2.0
   *
   * @param url  The URL of the page/file to open, or the email address.
   * Must use HTTP/HTTPS/file/mailto protocol. For example:
   *   "http://www.adobe.com"
   *   "https://github.com"
   *   "file:///C:/log.txt"
   *   "mailto:test@adobe.com"
   *
   * @return One of these error codes:\n
   *      <ul>\n
   *          <li>NO_ERROR - 0</li>\n
   *          <li>ERR_UNKNOWN - 1</li>\n
   *          <li>ERR_INVALID_PARAMS - 2</li>\n
   *          <li>ERR_INVALID_URL - 201</li>\n
   *      </ul>\n
   */
  openURLInDefaultBrowser(url: string): number {
    return cep.util.openURLInDefaultBrowser(url);
  };

  /**
   * Retrieves extension ID.
   *
   * Since 4.2.0
   *
   * @return extension ID.
   */
  getExtensionID<T extends string>(): T {
    return window.__adobe_cep__.getExtensionId();
  };

  /**
   * Retrieves the scale factor of screen.
   * On Windows platform, the value of scale factor might be different from operating system's scale factor,
   * since host application may use its self-defined scale factor.
   *
   * Since 4.2.0
   *
   * @return One of the following float number.
   *      <ul>\n
   *          <li> -1.0 when error occurs </li>\n
   *          <li> 1.0 means normal screen </li>\n
   *          <li> >1.0 means HiDPI screen </li>\n
   *      </ul>\n
   */
  getScaleFactor(): number {
    return window.__adobe_cep__.getScaleFactor();
  };

  /**
   * Set a handler to detect any changes of scale factor. This only works on Mac.
   *
   * Since 4.2.0
   *
   * @param handler   The function to be called when scale factor is changed.
   *
   */
  setScaleFactorChangedHandler(handler: () => void) {
    window.__adobe_cep__.setScaleFactorChangedHandler(handler);
  };


  /**
   * Retrieves current API version.
   *
   * Since 4.2.0
   *
   * @return ApiVersion object.
   *
   */
  getCurrentApiVersion(): ApiVersion {
    const apiVersion = JSON.parse(window.__adobe_cep__.getCurrentApiVersion());
    return apiVersion;
  };

  /**
   * Set panel flyout menu by an XML.
   *
   * Since 5.2.0
   *
   * Register a callback function for "com.adobe.csxs.events.flyoutMenuClicked" to get notified when a
   * menu item is clicked.
   * The "data" attribute of event is an object which contains "menuId" and "menuName" attributes.
   *
   * Register callback functions for "com.adobe.csxs.events.flyoutMenuOpened" and "com.adobe.csxs.events.flyoutMenuClosed"
   * respectively to get notified when flyout menu is opened or closed.
   *
   * @param menu     A XML string which describes menu structure.
   * An example menu XML:
   * <Menu>
   *   <MenuItem Id="menuItemId1" Label="TestExample1" Enabled="true" Checked="false"/>
   *   <MenuItem Label="TestExample2">
   *     <MenuItem Label="TestExample2-1" >
   *       <MenuItem Label="TestExample2-1-1" Enabled="false" Checked="true"/>
   *     </MenuItem>
   *     <MenuItem Label="TestExample2-2" Enabled="true" Checked="true"/>
   *   </MenuItem>
   *   <MenuItem Label="---" />
   *   <MenuItem Label="TestExample3" Enabled="false" Checked="false"/>
   * </Menu>
   *
   */
  setPanelFlyoutMenu(menu: string) {
    if ("string" != typeof menu) {
      return;
    }

    window.__adobe_cep__.invokeSync("setPanelFlyoutMenu", menu);
  };

  /**
   * Updates a menu item in the extension window's flyout menu, by setting the enabled
   * and selection status.
   *
   * Since 5.2.0
   *
   * @param menuItemLabel    The menu item label.
   * @param enabled        True to enable the item, false to disable it (gray it out).
   * @param checked        True to select the item, false to deselect it.
   *
   * @return false when the host application does not support this functionality (HostCapabilities.EXTENDED_PANEL_MENU is false).
   *         Fails silently if menu label is invalid.
   *
   * @see HostCapabilities.EXTENDED_PANEL_MENU
   */
  updatePanelMenuItem(menuItemLabel: string, enabled: boolean, checked: boolean): boolean {
    let ret = false;
    if (this.getHostCapabilities().EXTENDED_PANEL_MENU) {
      const itemStatus = new MenuItemStatus(menuItemLabel, enabled, checked);
      ret = window.__adobe_cep__.invokeSync("updatePanelMenuItem", JSON.stringify(itemStatus));
    }
    return ret;
  };


  /**
   * Set context menu by XML string.
   *
   * Since 5.2.0
   *
   * There are a number of conventions used to communicate what type of menu item to create and how it should be handled.
   * - an item without menu ID or menu name is disabled and is not shown.
   * - if the item name is "---" (three hyphens) then it is treated as a separator. The menu ID in this case will always be NULL.
   * - Checkable attribute takes precedence over Checked attribute.
   * - a PNG icon. For optimal display results please supply a 16 x 16px icon as larger dimensions will increase the size of the menu item.
   The Chrome extension contextMenus API was taken as a reference.
   https://developer.chrome.com/extensions/contextMenus
   * - the items with icons and checkable items cannot coexist on the same menu level. The former take precedences over the latter.
   *
   * @param menu      A XML string which describes menu structure.
   * @param callback  The callback function which is called when a menu item is clicked. The only parameter is the returned ID of clicked menu item.
   *
   * @description An example menu XML:
   * <Menu>
   *   <MenuItem Id="menuItemId1" Label="TestExample1" Enabled="true" Checkable="true" Checked="false" Icon="./image/small_16X16.png"/>
   *   <MenuItem Id="menuItemId2" Label="TestExample2">
   *     <MenuItem Id="menuItemId2-1" Label="TestExample2-1" >
   *       <MenuItem Id="menuItemId2-1-1" Label="TestExample2-1-1" Enabled="false" Checkable="true" Checked="true"/>
   *     </MenuItem>
   *     <MenuItem Id="menuItemId2-2" Label="TestExample2-2" Enabled="true" Checkable="true" Checked="true"/>
   *   </MenuItem>
   *   <MenuItem Label="---" />
   *   <MenuItem Id="menuItemId3" Label="TestExample3" Enabled="false" Checkable="true" Checked="false"/>
   * </Menu>
   */
  setContextMenu(menu: string, callback: (clikedMenuId: string) => unknown) {
    if ("string" != typeof menu) {
      return;
    }

    window.__adobe_cep__.invokeAsync("setContextMenu", menu, callback);
  };

  /**
   * Set context menu by JSON string.
   *
   * Since 6.0.0
   *
   * There are a number of conventions used to communicate what type of menu item to create and how it should be handled.
   * - an item without menu ID or menu name is disabled and is not shown.
   * - if the item label is "---" (three hyphens) then it is treated as a separator. The menu ID in this case will always be NULL.
   * - Checkable attribute takes precedence over Checked attribute.
   * - a PNG icon. For optimal display results please supply a 16 x 16px icon as larger dimensions will increase the size of the menu item.
   The Chrome extension contextMenus API was taken as a reference.
   * - the items with icons and checkable items cannot coexist on the same menu level. The former take precedences over the latter.
   https://developer.chrome.com/extensions/contextMenus
   *
   * @param menu      A JSON string which describes menu structure.
   * @param callback  The callback function which is called when a menu item is clicked. The only parameter is the returned ID of clicked menu item.
   *
   * @description An example menu JSON:
   *
   * {
   *      "menu": [
   *          {
   *              "id": "menuItemId1",
   *              "label": "testExample1",
   *              "enabled": true,
   *              "checkable": true,
   *              "checked": false,
   *              "icon": "./image/small_16X16.png"
   *          },
   *          {
   *              "id": "menuItemId2",
   *              "label": "testExample2",
   *              "menu": [
   *                  {
   *                      "id": "menuItemId2-1",
   *                      "label": "testExample2-1",
   *                      "menu": [
   *                          {
   *                              "id": "menuItemId2-1-1",
   *                              "label": "testExample2-1-1",
   *                              "enabled": false,
   *                              "checkable": true,
   *                              "checked": true
   *                          }
   *                      ]
   *                  },
   *                  {
   *                      "id": "menuItemId2-2",
   *                      "label": "testExample2-2",
   *                      "enabled": true,
   *                      "checkable": true,
   *                      "checked": true
   *                  }
   *              ]
   *          },
   *          {
   *              "label": "---"
   *          },
   *          {
   *              "id": "menuItemId3",
   *              "label": "testExample3",
   *              "enabled": false,
   *              "checkable": true,
   *              "checked": false
   *          }
   *      ]
   *  }
   *
   */
  setContextMenuByJSON(menu: string, callback: (clikedMenuId: string) => unknown) {
    if ("string" != typeof menu) {
      return;
    }

    window.__adobe_cep__.invokeAsync("setContextMenuByJSON", menu, callback);
  };

  /**
   * Updates a context menu item by setting the enabled and selection status.
   *
   * Since 5.2.0
   *
   * @param menuItemID    The menu item ID.
   * @param enabled        True to enable the item, false to disable it (gray it out).
   * @param checked        True to select the item, false to deselect it.
   */
  updateContextMenuItem(menuItemID: string, enabled: boolean, checked: boolean) {
    const itemStatus = new ContextMenuItemStatus(menuItemID, enabled, checked);
    return window.__adobe_cep__.invokeSync("updateContextMenuItem", JSON.stringify(itemStatus));
  };

  /**
   * Get the visibility status of an extension window.
   *
   * Since 6.0.0
   *
   * @return true if the extension window is visible; false if the extension window is hidden.
   */
  isWindowVisible(): boolean {
    return window.__adobe_cep__.invokeSync("isWindowVisible", "");
  };

  /**
   * Resize extension's content to the specified dimensions.
   * 1. Works with modal and modeless extensions in all Adobe products.
   * 2. Extension's manifest min/max size constraints apply and take precedence.
   * 3. For panel extensions
   *    3.1 This works in all Adobe products except:
   *        * Premiere Pro
   *        * Prelude
   *        * After Effects
   *    3.2 When the panel is in certain states (especially when being docked),
   *        it will not change to the desired dimensions even when the
   *        specified size satisfies min/max constraints.
   *
   * Since 6.0.0
   *
   * @param width  The new width
   * @param height The new height
   */
  resizeContent(width: number, height: number) {
    window.__adobe_cep__.resizeContent(width, height);
  };

  /**
   * Register the invalid certificate callback for an extension.
   * This callback will be triggered when the extension tries to access the web site that contains the invalid certificate on the main frame.
   * But if the extension does not call this function and tries to access the web site containing the invalid certificate, a default error page will be shown.
   *
   * Since 6.1.0
   *
   * @param callback the callback function
   */
  registerInvalidCertificateCallback(callback: () => void): any {
    return window.__adobe_cep__.registerInvalidCertificateCallback(callback);
  };

  /**
   * Register an interest in some key events to prevent them from being sent to the host application.
   *
   * This function works with modeless extensions and panel extensions.
   * Generally all the key events will be sent to the host application for these two extensions if the current focused element
   * is not text input or dropdown,
   * If you want to intercept some key events and want them to be handled in the extension, please call this function
   * in advance to prevent them being sent to the host application.
   *
   * Since 6.1.0
   *
   * @param keyEventsInterest      A JSON string describing those key events you are interested in. A null object or
   an empty string will lead to removing the interest
   *
   * This JSON string should be an array, each object has following keys:
   *
   * keyCode:  [Required] represents an OS system dependent virtual key code identifying
   *           the unmodified value of the pressed key.
   * ctrlKey:  [optional] a Boolean that indicates if the control key was pressed (true) or not (false) when the event occurred.
   * altKey:   [optional] a Boolean that indicates if the alt key was pressed (true) or not (false) when the event occurred.
   * shiftKey: [optional] a Boolean that indicates if the shift key was pressed (true) or not (false) when the event occurred.
   * metaKey:  [optional] (Mac Only) a Boolean that indicates if the Meta key was pressed (true) or not (false) when the event occurred.
   *                      On Macintosh keyboards, this is the command key. To detect Windows key on Windows, please use keyCode instead.
   * An example JSON string:
   *
   * [
   *     {
   *         "keyCode": 48
   *     },
   *     {
   *         "keyCode": 123,
   *         "ctrlKey": true
   *     },
   *     {
   *         "keyCode": 123,
   *         "ctrlKey": true,
   *         "metaKey": true
   *     }
   * ]
   *
   */
  registerKeyEventsInterest(keyEventsInterest: string): any {
    return window.__adobe_cep__.registerKeyEventsInterest(keyEventsInterest);
  };

  /**
   * Set the title of the extension window.
   * This function works with modal and modeless extensions in all Adobe products, and panel extensions in Photoshop, InDesign, InCopy, Illustrator, Flash Pro and Dreamweaver.
   *
   * Since 6.1.0
   *
   * @param title The window title.
   */
  setWindowTitle(title: string) {
    window.__adobe_cep__.invokeSync("setWindowTitle", title);
  };

  /**
   * Get the title of the extension window.
   * This function works with modal and modeless extensions in all Adobe products, and panel extensions in Photoshop, InDesign, InCopy, Illustrator, Flash Pro and Dreamweaver.
   *
   * Since 6.1.0
   *
   * @return The window title.
   */
  getWindowTitle(): string {
    return window.__adobe_cep__.invokeSync("getWindowTitle", "");
  };
}

/**
* Class CSEvent.
* You can use it to dispatch a standard CEP event.
*
* @return CSEvent object
*/
export class CSEvent {
  /**
   * Initializes new instance of CSEvent object.
   * You can use it to dispatch a standard CEP event.
   *
   * @param type        Event type.
   * @param scope       The scope of event, can be "GLOBAL" or "APPLICATION".
   * @param appId       The unique identifier of the application that generated the event. Optional.
   * @param extensionId The unique identifier of the extension that generated the event. Optional.
   */
  constructor(public type: string, public scope: string, public appId?: string, public extensionId?: string) {
  }

  /**
   * Event-specific data.
   */
  public data = "";
}

export function isCEP(): boolean {
    return window.__adobe_cep__ !== undefined;
}

export function getInterface(): CSInterface {
    if (isCEP()) {
        return new CSInterface();
    } else {
        throw new Error("Failed to initialize interface! Host is not Adobe app");
    }
}