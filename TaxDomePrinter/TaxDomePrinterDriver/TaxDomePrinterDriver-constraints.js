/// <reference path="TaxDomePrinterDriver-Intellisense.js" />

// --------------------------------------------------------------------------
// Note: To disable intellisense for Windows 8.1 APIs, please delete the line below
/// <reference path="TaxDomePrinterDriver-Intellisense-Windows8.1.js" />
// --------------------------------------------------------------------------

var pskNs = "http://schemas.microsoft.com/windows/2003/08/printing/printschemakeywords";
var pskV11Ns = "http://schemas.microsoft.com/windows/2013/05/printing/printschemakeywordsv11";
var psfNs = "http://schemas.microsoft.com/windows/2003/08/printing/printschemaframework";

function validatePrintTicket(printTicket, scriptContext) {
    /// <param name="printTicket" type="IPrintSchemaTicket">
    ///     Print ticket to be validated.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <returns type="Number" integer="true">
    ///     Integer value indicating validation status.
    ///         1 - Print ticket is valid and was not modified.
    ///         2 - Print ticket was modified to make it valid.
    ///         0 - Print ticket is invalid.
    /// </returns>

    // Below demonstrates correct usage of IPrintSchemaTicket2 APIs so that the script does not terminate
    // when running on a Windows 8 version of PrintConfig.dll.
    if (printSchemaApiHelpers.supportsIPrintSchemaTicket2(printTicket)) {
        var param = printTicket.GetParameterInitializer("JobCopiesAllDocuments");
    }
}

function completePrintCapabilities(printTicket, scriptContext, printCapabilities) {
    /// <param name="printTicket" type="IPrintSchemaTicket" mayBeNull="true">
    ///     If not 'null', the print ticket's settings are used to customize the print capabilities.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <param name="printCapabilities" type="IPrintSchemaCapabilities">
    ///     Print capabilities object to be customized.
    /// </param>

    // Below demonstrates correct usage of IPrintSchemaCapabilities2 APIs so that the script does not terminate
    // when running on a Windows 8 version of PrintConfig.dll.
    if (printSchemaApiHelpers.supportsIPrintSchemaCapabilities2(printCapabilities)) {
        var param = printCapabilities.GetParameterDefinition("JobCopiesAllDocuments");
    }

}

function convertPrintTicketToDevMode(printTicket, scriptContext, devModeProperties) {
    /// <param name="printTicket" type="IPrintSchemaTicket">
    ///     Print ticket to be converted to DevMode.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <param name="devModeProperties" type="IPrinterScriptablePropertyBag">
    ///     The DevMode property bag.
    /// </param>
}


function convertDevModeToPrintTicket(devModeProperties, scriptContext, printTicket) {
    /// <param name="devModeProperties" type="IPrinterScriptablePropertyBag">
    ///     The DevMode property bag.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <param name="printTicket" type="IPrintSchemaTicket">
    ///     Print ticket to be converted from the DevMode.
    /// </param>

}

var PrintSchemaConstrainedSetting = {
    PrintSchemaConstrainedSetting_None: 0,
    PrintSchemaConstrainedSetting_PrintTicket: 1,
    PrintSchemaConstrainedSetting_Admin: 2,
    PrintSchemaConstrainedSetting_Device: 3
};

var PrintSchemaParameterDataType = {
    PrintSchemaParameterDataType_Integer: 0,
    PrintSchemaParameterDataType_NumericString: 1,
    PrintSchemaParameterDataType_String: 2
};

var STREAM_SEEK = {
    STREAM_SEEK_SET: 0,
    STREAM_SEEK_CUR: 1,
    STREAM_SEEK_END: 2
};

var PrintSchemaSelectionType = {
    PrintSchemaSelectionType_PickOne: 0,
    PrintSchemaSelectionType_PickMany: 1
};


var printSchemaApiHelpers = {
    supportsIPrintSchemaCapabilities2: function (printCapabilities) {
        /// <summary>
        ///     Determines if the IPrintSchemaCapabilities2 APIs are supported on the 'printCapabilities' object.
        /// </summary>
        /// <param name="printCapabilities" type="IPrintSchemaCapabilities">
        ///     Print capabilities object.
        /// </param>
        /// <returns type="Boolean">
        ///     true - the interface APIs are supported.
        ///     false - the interface APIs are not supported.
        /// </returns>

        var supported = true;

        try {
            if (typeof printCapabilities.getParameterDefinition === "undefined") {
                supported = false;
            }
        }
        catch (exception) {
            supported = false;
        }

        return supported;
    },
    supportsIPrintSchemaTicket2: function (printTicket) {
        /// <summary>
        ///     Determines if the IPrintSchemaTicket2 APIs are supported on the 'printTicket' object.
        /// </summary>
        /// <param name="printTicket" type="IPrintSchemaTicket">
        ///     Print ticket object.
        /// </param>
        /// <returns type="Boolean">
        ///     true - the interface APIs are supported.
        ///     false - the interface APIs are not supported.
        /// </returns>

        var supported = true;

        try {
            if (typeof printTicket.getParameterInitializer === "undefined") {
                supported = false;
            }
        }
        catch (exception) {
            supported = false;
        }

        return supported;
    }
}