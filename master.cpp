/*
 * EtherCAT Debug Tool
 * A GUI tool for debugging EtherCAT networks using SOEM and ImGui
 */

#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <implot.h>
#include "imgui_main.hpp"
#include "fieldbus.h"
#include "soem/ec_main.h"
#include "soem/ec_type.h"
#include "soem/soem.h"

// Function declarations
void imgui_init(void);

static Fieldbus g_fieldbus;

// Global variables
static boolean adapter_opened = FALSE;
static boolean slaves_scanned = FALSE;
static boolean slaves_in_op   = FALSE;

// UI state variables
static int  selected_adapter = 0;
static char adapter_names[16][256];
static int  adapter_count              = 0;
static char selected_adapter_name[256] = {0};

// Debug variables
static bool show_debug_info = false;
static char debug_log[2048] = {0};

struct SDO_Object {
    uint16_t index;
    const char* name;
    uint16_t value;
    char write_buf[16];
    bool auto_refresh;
};

#include "e2m/output/ecat_master_data.c"

static bool auto_refresh_enabled = true;
static int refresh_interval_ms = 500;
static uint64_t last_refresh_time = 0;
bool should_refresh;

bool sdo_read_uint16(int slave, uint16_t index, uint16_t subindex, uint16_t* value)
{
    int size = 2;
    int wkc = ecx_SDOread(&g_fieldbus.context, slave, index, subindex, FALSE, &size, value, EC_TIMEOUTRXM);
    return (wkc > 0);
}

bool sdo_write_uint16(int slave, uint16_t index, uint16_t subindex, uint16_t value)
{
    int wkc = ecx_SDOwrite(&g_fieldbus.context, slave, index, subindex, FALSE, sizeof(value), &value, EC_TIMEOUTRXM);
    return (wkc > 0);
}

// Debug function to print state constants
void print_state_constants()
{
    printf("EtherCAT State Constants:\n");
    printf("EC_STATE_INIT = 0x%02X\n", EC_STATE_INIT);
    printf("EC_STATE_PRE_OP = 0x%02X\n", EC_STATE_PRE_OP);
    printf("EC_STATE_SAFE_OP = 0x%02X\n", EC_STATE_SAFE_OP);
    printf("EC_STATE_OPERATIONAL = 0x%02X\n", EC_STATE_OPERATIONAL);
    printf("EC_STATE_ACK = 0x%02X\n", EC_STATE_ACK);
}

// Function to scan available network adapters
void scan_adapters()
{
    ec_adaptert *adapter = NULL;
    ec_adaptert *head    = NULL;

    adapter_count = 0;
    head = adapter = ec_find_adapters();

    while (adapter != NULL && adapter_count < 16)
    {
        strncpy(adapter_names[adapter_count], adapter->name, sizeof(adapter_names[adapter_count]) - 1);
        adapter_names[adapter_count][sizeof(adapter_names[adapter_count]) - 1] = '\0';
        adapter_count++;
        adapter = adapter->next;
    }

    ec_free_adapters(head);
}

// Function to open selected adapter
bool open_adapter()
{
    if (selected_adapter >= 0 && selected_adapter < adapter_count)
    {
        strcpy(selected_adapter_name, adapter_names[selected_adapter]);

        if (ecx_init(&g_fieldbus.context, selected_adapter_name))
        {
            adapter_opened = TRUE;
            ecx_close(&g_fieldbus.context);
            fieldbus_initialize(&g_fieldbus, selected_adapter_name);

            printf("Adapter %s opened successfully\n", selected_adapter_name);
            return true;
        }
        else
        {
            printf("Failed to open adapter %s\n", selected_adapter_name);
            return false;
        }
    }

    return false;
}

// Function to close adapter
void close_adapter()
{
    if (adapter_opened)
    {
        // ecx_close(&g_fieldbus.context);
        adapter_opened = FALSE;
        slaves_scanned = FALSE;
        slaves_in_op   = FALSE;
        printf("Adapter closed\n");
    }
}

// Function to scan slaves
bool scan_slaves()
{
    if (!adapter_opened) return false;
    ecx_contextt *context;
    ec_groupt    *grp;
    int           i;
    context = &g_fieldbus.context;
    grp     = context->grouplist + g_fieldbus.group;

    printf("Initializing SOEM on '%s'... ", g_fieldbus.iface);
    if (!ecx_init(context, g_fieldbus.iface))
    {
        printf("no socket connection\n");
        return FALSE;
    }
    printf("done\n");
    if (ecx_config_init(context) > 0)
    {
        printf("%d slaves found\n", context->slavecount);

        ecx_config_map_group(context, g_fieldbus.map, 0);
        printf("mapped %dO+%dI bytes from %d segments", grp->Obytes, grp->Ibytes, grp->nsegments);
        if (grp->nsegments > 1)
        {
            /* Show how slaves are distributed */
            for (i = 0; i < grp->nsegments; ++i)
            {
                printf("%s%d", i == 0 ? " (" : "+", grp->IOsegment[i]);
            }
            printf(" slaves)");
        }
        printf("\n");

        printf("Configuring distributed clock... ");
        ecx_configdc(context);
        printf("done\n");

        printf("Waiting for all slaves in safe operational... ");
        ecx_statecheck(context, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);
        printf("done\n");

        printf("Send a roundtrip to make outputs in slaves happy... ");
        fieldbus_roundtrip(&g_fieldbus);
        printf("done\n");

        while (context->ecaterror)
        {
            printf("Error: %s", ecx_elist2string(context));
        }

        slaves_scanned = TRUE;
        printf("%d slaves found and configured\n", context->slavecount);

        // Print state constants for debugging
        print_state_constants();

        // First, read current states
        ecx_readstate(context);
        printf("Current slave states after configuration:\n");
        for (int i = 1; i <= context->slavecount; i++)
        {
            printf("Slave %d: State=%2x StatusCode=%4x\n",
                   i,
                   context->slavelist[i].state,
                   context->slavelist[i].ALstatuscode);
        }

        // Try to request INIT state explicitly
        printf("Requesting INIT state...\n");
        ecx_writestate(context, 1);
        usleep(200000);

        // Check states after INIT request
        ecx_readstate(context);
        printf("Slave states after INIT request:\n");
        for (int i = 1; i <= context->slavecount; i++)
        {
            printf("Slave %d: State=%2x StatusCode=%4x\n",
                   i,
                   context->slavelist[i].state,
                   context->slavelist[i].ALstatuscode);
        }

        return true;
    }
    else
    {
        printf("No slaves found!\n");
        return false;
    }
}


// Function to manually control slave states
bool manual_state_control(int target_state)
{
    if (!slaves_scanned) return false;

    printf("Manually requesting state 0x%02X for all slaves...\n", target_state);

    // First, read current state
    ecx_readstate(&g_fieldbus.context);
    printf("Current master state: 0x%02X\n", g_fieldbus.context.slavelist[0].state);

    // Request target state for all slaves
    // Set master to target state first
    g_fieldbus.context.slavelist[0].state = target_state;

    // If requesting OPERATIONAL state, send process data first
    if (target_state == EC_STATE_OPERATIONAL)
    {
        printf("Sending process data cycles before requesting OP state...\n");
        for (int cycle = 0; cycle < 10; cycle++)
        {
            ecx_send_processdata(&g_fieldbus.context);
            int wkc = ecx_receive_processdata(&g_fieldbus.context, EC_TIMEOUTRET);
            printf("Cycle %d: WKC=%d\n", cycle, wkc);
        }

        // Now request the state
        ecx_writestate(&g_fieldbus.context, 0);
        usleep(1000000);

        // Continuously send process data while waiting for OP state
        printf("Waiting for OP state while sending process data...\n");
        int chk = 100;
        do
        {
            ecx_send_processdata(&g_fieldbus.context);
            ecx_receive_processdata(&g_fieldbus.context, EC_TIMEOUTRET);
            ecx_statecheck(&g_fieldbus.context, 0, target_state, 50000);
            chk--;
        } while (chk-- && (g_fieldbus.context.slavelist[0].state != target_state));
    }
    else
    {
        // For other states, just request and wait
        ecx_writestate(&g_fieldbus.context, 0);
        printf("State request sent, waiting for response...\n");
        usleep(500000);

        // Check state
        int wkc = ecx_statecheck(&g_fieldbus.context, 0, target_state, EC_TIMEOUTSTATE * 5);
        printf("State check result: wkc=%d\n", wkc);
    }

    printf("State check result:\n");
    ecx_readstate(&g_fieldbus.context);
    for (int i = 1; i <= g_fieldbus.context.slavecount; i++)
    {
        printf("Slave %d: State=%2x StatusCode=%4x\n",
               i,
               g_fieldbus.context.slavelist[i].state,
               g_fieldbus.context.slavelist[i].ALstatuscode);
    }

    bool success = (g_fieldbus.context.slavelist[0].state == target_state);
    if (success)
    {
        printf("Successfully changed to state %d\n", target_state);

        // If we reached OPERATIONAL state, start process data exchange
        if (target_state == EC_STATE_OPERATIONAL)
        {
            slaves_in_op = TRUE;
            int wkc      = ecx_receive_processdata(&g_fieldbus.context, EC_TIMEOUTRET);
            printf("Process data exchange started, WKC=%d\n", wkc);
        }
        else
        {
            slaves_in_op = FALSE;
        }
    }
    else
    {
        printf("Failed to change to state %d\n", target_state);
    }

    return success;
}

// Main UI function
void main_window(void)
{
    // Main window
    if (ImGui::Begin("EtherCAT Debug Tool"))
    {
        // Adapter selection section
        ImGui::Text("Network Adapter");
        ImGui::Separator();

        if (ImGui::Button("Scan Adapters"))
        {
            scan_adapters();
        }

        ImGui::SameLine();
        ImGui::Text("Found %d adapters", adapter_count);

        if (adapter_count > 0)
        {
            const char *adapter_items[16];
            for (int i = 0; i < adapter_count; i++)
            {
                adapter_items[i] = adapter_names[i];
            }

            ImGui::Combo("Select Adapter", &selected_adapter, adapter_items, adapter_count);

            ImGui::SameLine();
            if (!adapter_opened)
            {
                if (ImGui::Button("Open"))
                {
                    open_adapter();
                }
            }
            else
            {
                if (ImGui::Button("Close"))
                {
                    close_adapter();
                }
            }
        }

        ImGui::Spacing();

        // Slave scanning section
        ImGui::Text("Slave Management");
        ImGui::Separator();

        if (adapter_opened)
        {
            if (!slaves_scanned)
            {
                if (ImGui::Button("Scan Slaves"))
                {
                    scan_slaves();
                }
            }
            else
            {
                ImGui::Text("Slaves: %d", g_fieldbus.context.slavecount);

                ImGui::SameLine();
                if (ImGui::Button("Clear Error"))
                {
                    // Clear error state by reading current state
                    printf("Clearing error states...\n");
                    ecx_readstate(&g_fieldbus.context);
                    for (int i = 1; i <= g_fieldbus.context.slavecount; i++)
                    {
                        if (g_fieldbus.context.slavelist[i].state & EC_STATE_ACK)
                        {
                            printf("Slave %d has error state 0x%02X, StatusCode=%4x\n",
                                   i,
                                   g_fieldbus.context.slavelist[i].state,
                                   g_fieldbus.context.slavelist[i].ALstatuscode);
                        }
                    }
                }

                ImGui::NewLine();
                if (ImGui::Button("BOOT"))
                {
                    manual_state_control(EC_STATE_BOOT);
                }
                ImGui::SameLine();
                if (ImGui::Button("INIT"))
                {
                    manual_state_control(EC_STATE_INIT);
                }
                ImGui::SameLine();
                if (ImGui::Button("PRE_OP"))
                {
                    manual_state_control(EC_STATE_PRE_OP);
                }
            }
        }
        else
        {
            ImGui::Text("Please open an adapter first");
        }

        ImGui::Spacing();

        if (slaves_scanned)
        {
            ImGui::Text("SDO Debug Panel");
            ImGui::Separator();

            if (ImGui::BeginTable("SDO_Table", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Index");
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Value");
                ImGui::TableSetupColumn("Write");
                ImGui::TableSetupColumn("Actions");
                ImGui::TableHeadersRow();

                for (int i = 0; i < sdo_count; i++)
                {
                    ImGui::TableNextRow();
                    SDO_Object& obj = sdo_objects[i];

                    if (should_refresh && obj.auto_refresh)
                    {
                        sdo_read_uint16(1, obj.index, 0, &obj.value);
                    }

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", i);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("0x%04X", obj.index);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", obj.name);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextColored(ImVec4(0,1,0,1), "%u", obj.value);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::PushItemWidth(80);
                    ImGui::InputTextWithHint(("Value#" + std::to_string(i)).c_str(), "value", obj.write_buf, sizeof(obj.write_buf), ImGuiInputTextFlags_CharsDecimal);
                    ImGui::PopItemWidth();

                    ImGui::TableSetColumnIndex(5);
                    if (ImGui::Button(("Write##" + std::to_string(i)).c_str()))
                    {
                        uint16_t val = (uint16_t)atoi(obj.write_buf);
                        if (sdo_write_uint16(1, obj.index, 0, val))
                        {
                            obj.value = val;
                            strcpy(obj.write_buf, "");
                            printf("SDO Write 0x%04X = %d OK\n", obj.index, val);
                        }
                    }
                    ImGui::SameLine();
                    ImGui::Checkbox(("Auto##" + std::to_string(i)).c_str(), &obj.auto_refresh);
                }
                should_refresh = false;
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();
}

void ImGui::mystyle(void)
{
    ImGuiIO    &io    = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    style.FramePadding.x = 10;
    style.FramePadding.y = 3;

    style.FrameRounding = 3;
    style.GrabRounding  = 10;

    // style.TabBorderSize = 5;
    // style.WindowBorderSize = 0;
    style.WindowRounding  = 3;
    style.WindowPadding.x = 10;
    style.WindowPadding.y = 10;

    style.ItemSpacing.x = 10;
    style.ItemSpacing.y = 5;

    // Initialize UI state
    scan_adapters();
}

namespace ImGui
{
    void mywindow(void);
}

void ImGui::mywindow(void)
{
    main_window();
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
}

void* thread_func_ecat(void *arg)
{
    for (;;)
    {
        should_refresh = true;
        usleep(5000);
    }
    return NULL;
}

void *thread_func_ui(void *arg)
{
    imgui_init();
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("EtherCAT Debug Tool\n");
    printf("A GUI tool for debugging EtherCAT networks\n");
    pthread_t tid1, tid2;
    int id1 = 1, id2 = 2;

    // create thread
    if (pthread_create(&tid1, NULL, thread_func_ui, &id1) != 0) {
        perror("thread_func_ui is created.");
        return 1;
    }

    if (pthread_create(&tid2, NULL, thread_func_ecat, &id2) != 0) {
        perror("thread_func_ecat is created.");
        return 1;
    }

    // wait for threads to finish
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}
