// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 Intel Corporation. All Rights Reserved.

#include <jni.h>
#include <memory>
#include <vector>
#include "error.h"
#include <iostream>
#include <fstream>
#include <string>

#include "../../../include/librealsense2/rs.h"
#include "../../../include/librealsense2/hpp/rs_device.hpp"
#include "../../api.h"
#include "../../../common/parser.hpp"

using namespace std;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_intel_realsense_librealsense_Device_nSupportsInfo(JNIEnv *env, jclass type, jlong handle,
                                                           jint info) {
    rs2_error *e = NULL;
    auto rv = rs2_supports_device_info(reinterpret_cast<const rs2_device *>(handle),
                                       static_cast<rs2_camera_info>(info), &e);
    handle_error(env, e);
    return rv > 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_intel_realsense_librealsense_Device_nGetInfo(JNIEnv *env, jclass type, jlong handle,
                                                      jint info) {
    rs2_error *e = NULL;
    const char* rv = rs2_get_device_info(reinterpret_cast<const rs2_device *>(handle),
                                         static_cast<rs2_camera_info>(info), &e);
    handle_error(env, e);
    if (NULL == rv)
        rv = "";
    return env->NewStringUTF(rv);
}

extern "C" JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_Device_nRelease(JNIEnv *env, jclass type, jlong handle) {
    rs2_delete_device(reinterpret_cast<rs2_device *>(handle));
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_com_intel_realsense_librealsense_Device_nQuerySensors(JNIEnv *env, jclass type, jlong handle) {
    rs2_error *e = nullptr;
    std::shared_ptr<rs2_sensor_list> list(
            rs2_query_sensors(reinterpret_cast<const rs2_device *>(handle), &e),
            rs2_delete_sensor_list);
    handle_error(env, e);

    auto size = rs2_get_sensors_count(list.get(), &e);
    handle_error(env, e);

    std::vector<rs2_sensor *> sensors;
    for (auto i = 0; i < size; i++) {
        auto s = rs2_create_sensor(list.get(), i, &e);
        handle_error(env, e);
        sensors.push_back(s);
    }
    jlongArray rv = env->NewLongArray(sensors.size());
    env->SetLongArrayRegion(rv, 0, sensors.size(), reinterpret_cast<const jlong *>(sensors.data()));
    return rv;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_Updatable_nEnterUpdateState(JNIEnv *env, jclass type,
                                                                  jlong handle) {
    rs2_error *e = NULL;
    rs2_enter_update_state(reinterpret_cast<const rs2_device *>(handle), &e);
    handle_error(env, e);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_Updatable_nUpdateFirmwareUnsigned(JNIEnv *env,
                                                                        jobject instance,
                                                                        jlong handle,
                                                                        jbyteArray image_,
                                                                        jint update_mode) {
    jbyte *image = env->GetByteArrayElements(image_, NULL);
    auto length = env->GetArrayLength(image_);
    rs2_error *e = NULL;
    jclass cls = env->GetObjectClass(instance);
    jmethodID id = env->GetMethodID(cls, "onProgress", "(F)V");
    auto cb = [&](float progress){ env->CallVoidMethod(instance, id, progress); };
    rs2_update_firmware_unsigned_cpp(reinterpret_cast<const rs2_device *>(handle), image, length,
                                     new rs2::update_progress_callback<decltype(cb)>(cb), update_mode, &e);
    handle_error(env, e);
    env->ReleaseByteArrayElements(image_, image, 0);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_intel_realsense_librealsense_Updatable_nCreateFlashBackup(JNIEnv *env, jobject instance,
                                                                   jlong handle) {
    rs2_error* e = NULL;
    jclass cls = env->GetObjectClass(instance);
    jmethodID id = env->GetMethodID(cls, "onProgress", "(F)V");
    auto cb = [&](float progress){ env->CallVoidMethod(instance, id, progress); };

    std::shared_ptr<const rs2_raw_data_buffer> raw_data_buffer(
            rs2_create_flash_backup_cpp(reinterpret_cast<rs2_device *>(handle), new rs2::update_progress_callback<decltype(cb)>(cb), &e),
            [](const rs2_raw_data_buffer* buff){ if(buff) delete buff;});
    handle_error(env, e);

    jbyteArray rv = env->NewByteArray(raw_data_buffer->buffer.size());
    env->SetByteArrayRegion(rv, 0, raw_data_buffer->buffer.size(),
        reinterpret_cast<const jbyte *>(raw_data_buffer->buffer.data()));
    return rv;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_UpdateDevice_nUpdateFirmware(JNIEnv *env, jobject instance,
                                                                   jlong handle,
                                                                   jbyteArray image_) {
    jbyte *image = env->GetByteArrayElements(image_, NULL);
    auto length = env->GetArrayLength(image_);
    rs2_error *e = NULL;
    jclass cls = env->GetObjectClass(instance);
    jmethodID id = env->GetMethodID(cls, "onProgress", "(F)V");
    auto cb = [&](float progress){ env->CallVoidMethod(instance, id, progress); };
    rs2_update_firmware_cpp(reinterpret_cast<const rs2_device *>(handle), image, length,
                            new rs2::update_progress_callback<decltype(cb)>(cb), &e);
    handle_error(env, e);
    env->ReleaseByteArrayElements(image_, image, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_intel_realsense_librealsense_Device_nHardwareReset(JNIEnv *env, jclass type,
                                                            jlong handle) {
    rs2_error *e = NULL;
    rs2_hardware_reset(reinterpret_cast<const rs2_device *>(handle), &e);
    handle_error(env, e);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_intel_realsense_librealsense_Device_nIsDeviceExtendableTo(JNIEnv *env, jclass type,
                                                                   jlong handle, jint extension) {
    rs2_error *e = NULL;
    int rv = rs2_is_device_extendable_to(reinterpret_cast<const rs2_device *>(handle),
                                         static_cast<rs2_extension>(extension), &e);
    handle_error(env, e);
    return rv > 0;
}

std::vector<uint8_t> send_and_receive_raw_data(JNIEnv *env, rs2_device * dev, const std::vector<uint8_t>& input)
{
    std::vector<uint8_t> results;

    rs2_error *e = NULL;
    std::shared_ptr<const rs2_raw_data_buffer> list(
            rs2_send_and_receive_raw_data(dev, (void*)input.data(), (uint32_t)input.size(), &e),
            rs2_delete_raw_data);
    handle_error(env, e);

    auto size = rs2_get_raw_data_size(list.get(), &e);
    handle_error(env, e);

    auto start = rs2_get_raw_data(list.get(), &e);

    results.insert(results.begin(), start, start + size);

    return results;
}

std::string hex_mode(JNIEnv *env, rs2_device * dev, const std::string& line)
{
    std::vector<uint8_t> raw_data;
    std::stringstream ss(line);
    std::string word;
    while (ss >> word)
    {
        std::stringstream converter;
        int temp;
        converter << std::hex << word;
        converter >> temp;
        raw_data.push_back(temp);
    }
    if (raw_data.empty())
        throw std::runtime_error("Wrong input!");

    auto result = send_and_receive_raw_data(env, dev, raw_data);

    std::stringstream rv;
    for (auto& elem : result)
        rv << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(elem) << " ";

    return rv.str();
}

std::vector<uint8_t> build_raw_command_data(const command& command, const std::vector<std::string>& params)
{
    if (params.size() > command.parameters.size() && !command.is_cmd_write_data)
        throw std::runtime_error("Input string was not in a correct format!");

    std::vector<parameter> vec_parameters;
    for (auto param_index = 0; param_index < params.size(); ++param_index)
    {
        auto is_there_write_data = param_index >= int(command.parameters.size());
        auto name = (is_there_write_data) ? "" : command.parameters[param_index].name;
        auto is_reverse_bytes = (is_there_write_data) ? false : command.parameters[param_index].is_reverse_bytes;
        auto is_decimal = (is_there_write_data) ? false : command.parameters[param_index].is_decimal;
        auto format_length = (is_there_write_data) ? -1 : command.parameters[param_index].format_length;
        vec_parameters.push_back(parameter(name, params[param_index], is_decimal, is_reverse_bytes, format_length));
    }

    std::vector<uint8_t> raw_data;
    encode_raw_data_command(command, vec_parameters, raw_data);
    return raw_data;
}

std::string xml_mode(const std::string& line, const commands_xml& cmd_xml, std::map<std::string,
        xml_parser_function>& format_type_to_lambda, JNIEnv *env, jlong handle)
{
    try {
        std::vector<string> tokens;
        std::stringstream ss(line);
        std::string word;
        while (ss >> word) {
            std::stringstream converter;
            converter << hex << word;
            tokens.push_back(word);
        }

        if (tokens.empty())
            throw std::runtime_error("Wrong input!");

        auto command_str = tokens.front();
        auto it = cmd_xml.commands.find(command_str);
        if (it == cmd_xml.commands.end())
            throw std::runtime_error("Command not found!");

        auto command = it->second;
        std::vector<std::string> params;
        for (auto i = 1; i < tokens.size(); ++i)
            params.push_back(tokens[i]);

        auto raw_data = build_raw_command_data(command, params);

        auto result = send_and_receive_raw_data(env, reinterpret_cast<rs2_device *>(handle),
                                                raw_data);

        unsigned returned_opcode = *result.data();
        // check returned opcode
        if (command.op_code != returned_opcode) {
            std::stringstream msg;
            msg << "OpCodes do not match! Sent 0x" << hex << command.op_code << " but received 0x"
                << hex << (returned_opcode) << "!";
            throw std::runtime_error(msg.str());
        }

        std::string rv;
        decode_string_from_raw_data(command, cmd_xml.custom_formatters, result.data(),
                                    result.size(), rv, format_type_to_lambda);
        return rv;
    }catch(std::exception& e){
        return e.what();
    }
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_intel_realsense_librealsense_DebugProtocol_nSendAndReceiveRawData(JNIEnv *env, jclass type,
                                                                           jlong handle,
                                                                           jbyteArray buffer_) {
    jbyte *buffer = env->GetByteArrayElements(buffer_, NULL);
    jsize length = env->GetArrayLength(buffer_);
    std::vector<uint8_t> buff(reinterpret_cast<uint8_t*>(buffer), reinterpret_cast<uint8_t*>(buffer) + length);
    auto ret = send_and_receive_raw_data(env, reinterpret_cast<rs2_device *>(handle), buff);
    env->ReleaseByteArrayElements(buffer_, buffer, 0);
    jbyteArray rv = env->NewByteArray(ret.size());
    env->SetByteArrayRegion (rv, 0, ret.size(), reinterpret_cast<const jbyte *>(ret.data()));
    return rv;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_intel_realsense_librealsense_DebugProtocol_nSendAndReceiveData(JNIEnv *env, jclass clazz,
                                                                        jlong handle,
                                                                        jstring filePath,
                                                                        jstring command) {
    const char *file_path = env->GetStringUTFChars(filePath, 0);
    const char *line = env->GetStringUTFChars(command, 0);

    std::map<string, xml_parser_function> format_type_to_lambda;
    commands_xml cmd_xml;

    std::string result = "failed to open commands file";
    bool sts = parse_xml_from_file(file_path, cmd_xml);
    if(sts){
        update_format_type_to_lambda(format_type_to_lambda);

        result = xml_mode(line, cmd_xml, format_type_to_lambda, env, handle);
    }

    env->ReleaseStringUTFChars(command, line);
    env->ReleaseStringUTFChars(filePath, file_path);

    jbyteArray rv = env->NewByteArray(result.size());
    env->SetByteArrayRegion(rv, 0, result.size(),
                            reinterpret_cast<const jbyte *>(result.c_str()));
    return rv;
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_intel_realsense_librealsense_DebugProtocol_nGetCommands(JNIEnv *env, jclass clazz,
                                                                 jstring filePath) {
    const char *file_path = env->GetStringUTFChars(filePath, 0);

    jobjectArray rv;
    commands_xml cmd_xml;
    bool sts = parse_xml_from_file(file_path, cmd_xml);
    if(sts)
    {
        rv = (jobjectArray)env->NewObjectArray(cmd_xml.commands.size(), env->FindClass("java/lang/String"),env->NewStringUTF(""));
        int i = 0;
        for(auto&& c : cmd_xml.commands)
            env->SetObjectArrayElement(rv, i++, env->NewStringUTF(c.first.c_str()));
    }
    env->ReleaseStringUTFChars(filePath, file_path);

    return(rv);
}
